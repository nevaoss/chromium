// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/webgpu/external_image_utils.h"

#include "third_party/blink/renderer/core/html/canvas/canvas_rendering_context_host.h"
#include "third_party/blink/renderer/core/html/canvas/element_image.h"
#include "third_party/blink/renderer/core/html/canvas/html_canvas_element.h"
#include "third_party/blink/renderer/core/html/canvas/image_data.h"
#include "third_party/blink/renderer/core/html/html_image_element.h"
#include "third_party/blink/renderer/core/imagebitmap/image_bitmap.h"
#include "third_party/blink/renderer/core/offscreencanvas/offscreen_canvas.h"
#include "third_party/blink/renderer/modules/webcodecs/video_frame.h"
#include "third_party/blink/renderer/platform/graphics/gpu/image_extractor.h"
#include "third_party/blink/renderer/platform/graphics/skia/skia_utils.h"
#include "third_party/blink/renderer/platform/graphics/unaccelerated_static_bitmap_image.h"

namespace blink {
namespace {

std::optional<ExternalImageSource> GetExternalImageSourceFromCanvasImageSource(
    CanvasImageSource* source,
    ExternalImageDstInfo dst_info,
    ExceptionState& exception_state) {
  CHECK(!source->IsNeutered() && !source->IsPlaceholder());

  // Canvas element contains cross-origin data and may not be loaded
  if (source->WouldTaintOrigin()) {
    exception_state.ThrowSecurityError(
        "The external image is tainted by cross-origin data.");
    return {};
  }

  ExternalImageSource result;

  // HTMLCanvasElement and OffscreenCanvas won't care image orientation. But for
  // ImageBitmap, use kRespectImageOrientation will make ElementSize() behave
  // as Size().
  gfx::SizeF image_size = source->ElementSize(
      gfx::SizeF(),  // It will be ignored and won't affect size.
      kRespectImageOrientation);

  // The alpha op will happen at CopyTextureForBrowser() and
  // CopyContentFromCPU(). This will help combine more transforms (e.g. flipY,
  // color-space) into a single blit.
  // TODO(https://crbug.com/40760113): Ensure unpremultiplied images will live
  // on GPU if possible.
  SourceImageStatus source_image_status = kInvalidSourceImageStatus;
  auto image_for_canvas =
      source->GetSourceImageForCanvas(&source_image_status, image_size);
  if (source_image_status != kNormalSourceImageStatus) {
    // Canvas back resource is broken, zero size, incomplete or invalid.
    // but developer can do nothing. Return nullptr and issue an noop.
    return {};
  }

  // TODO(https://crbug.com/40278208): It would be better if
  // GetSourceImageForCanvas() would always return a StaticBitmapImage.
  sk_sp<SkImage> sk_image = nullptr;
  bool image_is_default_orientation = image_for_canvas->HasDefaultOrientation();
  if (auto* image = DynamicTo<StaticBitmapImage>(image_for_canvas.get())) {
    if (image_is_default_orientation) {
      result.image = image;
    } else {
      // Handle non default orientation for StaticBitmapImage and ensure
      // it is not texture backed.
      sk_image = image->PaintImageForCurrentFrame().GetSwSkImage();

      if (!sk_image) {
        return {};
      }
    }
  } else {
    // HTMLImageElement input.
    // Below logic refs to ImageBitmap creation with ImageElementBase.
    // ImageExtractor recruit ImageDecoder to do decoder when:
    // - image is a BitmapImage, it usually happens when image contains coded
    // data.
    //   e.g. loaded image files *.png, *.jpg, *.bmp, *.ico, *.webp, *.avif,
    //   *.gif.
    // - alphaType, colorSpace are not equal to dst. Issuing a redecode to
    // generate
    //   required results.
    ImageExtractor image_extractor(
        image_for_canvas.get(),
        dst_info.premultiplied_alpha ? kPremul_SkAlphaType
                                     : kUnpremul_SkAlphaType,
        PredefinedColorSpaceToSkColorSpace(dst_info.color_space));
    sk_image = image_extractor.GetSkImage();

    if (!sk_image) {
      return {};
    }
    // It is possible that some HTMLImageElement contains content which cannot
    // be decoded. e.g svg files. Using this path to handle them by converting
    // it to SkBitmap first and raster it.
    if (sk_image->isLazyGenerated()) {
      SkBitmap bitmap;
      auto image_info = sk_image->imageInfo();
      bitmap.allocPixels(image_info, image_info.minRowBytes());
      if (!sk_image->readPixels(bitmap.pixmap(), 0, 0)) {
        return {};
      }

      sk_image = SkImages::RasterFromBitmap(bitmap);
    }
  }

  if (sk_image) {
    CHECK(!result.image);

    // Create UnacceleratedStaticBitmapImage to create a most suitable
    // PaintImageBuilder. Use the builder to create PaintImage internally.
    // Store the orientation metadata but no transforms apply to the content.
    auto image = UnacceleratedStaticBitmapImage::Create(
        std::move(sk_image), image_for_canvas->Orientation());

    // Recruit Image::ResizeAndOrientImage() to apply transformation based on
    // orientation metadata. This API helps rotate contents based on orientation
    // metadata. After the transformation, reading content in default
    // orientation get the transformed results. Recreate unaccelerated static
    // bitmap with the transformed content with default orientation for post
    // processing.
    if (!image_is_default_orientation) {
      PaintImage paint_image = image->PaintImageForCurrentFrame();
      paint_image = Image::ResizeAndOrientImage(
          paint_image, image_for_canvas->Orientation(), gfx::Vector2dF(1, 1), 1,
          kInterpolationNone);

      // Have default orientation now.
      image = UnacceleratedStaticBitmapImage::Create(std::move(paint_image));
    }

    result.image = image;
  }

  result.width = static_cast<uint32_t>(result.image->width());
  result.height = static_cast<uint32_t>(result.image->height());
  return {result};
}

std::optional<ExternalImageSource>
GetExternalImageSourceFromCanvasRenderingContextHost(
    CanvasRenderingContextHost* canvas,
    ExternalImageDstInfo dst_info,
    ExceptionState& exception_state) {
  if (!(canvas->IsWebGL() || canvas->IsRenderingContext2D() ||
        canvas->IsWebGPU() || canvas->IsImageBitmapRenderingContext())) {
    exception_state.ThrowDOMException(
        DOMExceptionCode::kOperationError,
        "CopyExternalImageToTexture doesn't support canvas without rendering "
        "context");
    return {};
  }

  return GetExternalImageSourceFromCanvasImageSource(canvas, dst_info,
                                                     exception_state);
}

}  // anonymous namespace

std::optional<ExternalImageSource> GetExternalImageSourceFrom(
    HTMLVideoElement* video,
    ExternalImageDstInfo dst_info,
    ExceptionState& exception_state) {
  auto external_texture_source =
      GetExternalTextureSourceFromVideoElement(video, exception_state);
  if (!external_texture_source.valid) {
    return {};
  }
  CHECK(external_texture_source.media_video_frame);

  ExternalImageSource result;

  // Use display size to handle rotated video frame.
  auto natural_size = external_texture_source.media_video_frame->natural_size();
  const auto transform = external_texture_source.media_video_frame->metadata()
                             .transformation.value_or(media::kNoTransformation);
  if (transform == media::kNoTransformation ||
      transform.rotation == media::VIDEO_ROTATION_0 ||
      transform.rotation == media::VIDEO_ROTATION_180) {
    result.width = static_cast<uint32_t>(natural_size.width());
    result.height = static_cast<uint32_t>(natural_size.height());
  } else {
    result.width = static_cast<uint32_t>(natural_size.height());
    result.height = static_cast<uint32_t>(natural_size.width());
  }

  result.external_texture_source = std::move(external_texture_source);
  return {result};
}

std::optional<ExternalImageSource> GetExternalImageSourceFrom(
    ImageData* image_data,
    ExternalImageDstInfo dst_info,
    ExceptionState& exception_state) {
  // TODO(https://crbug.com/40278208): Avoid extra copy.
  SkPixmap image_data_pixmap = image_data->GetSkPixmap();
  SkImageInfo info = image_data_pixmap.info().makeColorType(kN32_SkColorType);
  size_t image_pixels_size = info.computeMinByteSize();
  if (SkImageInfo::ByteSizeOverflowed(image_pixels_size)) {
    return {};
  }
  sk_sp<SkData> image_pixels = TryAllocateSkData(image_pixels_size);
  if (!image_pixels) {
    return {};
  }
  if (!image_data_pixmap.readPixels(info, image_pixels->writable_data(),
                                    info.minRowBytes(), 0, 0)) {
    return {};
  }

  ExternalImageSource result;
  result.image = StaticBitmapImage::Create(std::move(image_pixels), info,
                                           gfx::HDRMetadata());
  result.width = static_cast<uint32_t>(image_data->width());
  result.height = static_cast<uint32_t>(image_data->height());
  return {result};
}

std::optional<ExternalImageSource> GetExternalImageSourceFrom(
    VideoFrame* frame,
    ExternalImageDstInfo dst_info,
    ExceptionState& exception_state) {
  auto external_texture_source =
      GetExternalTextureSourceFromVideoFrame(frame, exception_state);
  if (!external_texture_source.valid) {
    return {};
  }
  CHECK(external_texture_source.media_video_frame);

  ExternalImageSource result;
  result.external_texture_source = external_texture_source;
  result.width = frame->displayWidth();
  result.height = frame->displayHeight();
  return {result};
}

std::optional<ExternalImageSource> GetExternalImageSourceFrom(
    HTMLCanvasElement* canvas,
    ExternalImageDstInfo dst_info,
    ExceptionState& exception_state) {
  if (canvas->IsPlaceholder()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      "Cannot copy from a canvas that has had "
                                      "transferControlToOffscreen() called.");
    return {};
  }

  return GetExternalImageSourceFromCanvasRenderingContextHost(canvas, dst_info,
                                                              exception_state);
}

std::optional<ExternalImageSource> GetExternalImageSourceFrom(
    OffscreenCanvas* canvas,
    ExternalImageDstInfo dst_info,
    ExceptionState& exception_state) {
  if (canvas->IsNeutered()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      "OffscreenCanvas has been transferred.");
    return {};
  }

  return GetExternalImageSourceFromCanvasRenderingContextHost(canvas, dst_info,
                                                              exception_state);
}

std::optional<ExternalImageSource> GetExternalImageSourceFrom(
    HTMLImageElement* image,
    ExternalImageDstInfo dst_info,
    ExceptionState& exception_state) {
  return GetExternalImageSourceFromCanvasImageSource(image, dst_info,
                                                     exception_state);
}

std::optional<ExternalImageSource> GetExternalImageSourceFrom(
    ImageBitmap* image,
    ExternalImageDstInfo dst_info,
    ExceptionState& exception_state) {
  if (image->IsNeutered()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      "ImageBitmap has been detached.");
    return {};
  }

  return GetExternalImageSourceFromCanvasImageSource(image, dst_info,
                                                     exception_state);
}

}  // namespace blink

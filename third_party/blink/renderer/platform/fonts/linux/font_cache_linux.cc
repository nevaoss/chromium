/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "third_party/blink/renderer/platform/fonts/font_cache.h"

#include "build/build_config.h"
#include "base/no_destructor.h"
#include "third_party/blink/public/platform/linux/web_sandbox_support.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/renderer/platform/fonts/font_fallback_priority.h"
#include "third_party/blink/renderer/platform/fonts/font_platform_data.h"
#include "third_party/blink/renderer/platform/fonts/simple_font_data.h"
#include "ui/gfx/font_fallback_linux.h"

#if BUILDFLAG(IS_NEVA_APPRUNTIME)
#include <unicode/uscript.h>
#endif

namespace blink {

#if BUILDFLAG(IS_NEVA_APPRUNTIME)
static const char* const kUnicodeRangeToLangTable[] = {
    0,    "el", "tr", "he", "ar", 0, "th", "ko", "ja", "zh-CN", "zh-TW",
    "hi", "ta", "hy", "bn", 0,    0, "ka", "gu", "pa", "km",    "ml"};

// Map ICU script codes to our lang table indices.
static unsigned char ScriptToRange(UScriptCode script) {
  switch (script) {
    case USCRIPT_GREEK: return 1;
    case USCRIPT_LATIN: return 2; // tr is Latin-based but commonly mapped here
    case USCRIPT_HEBREW: return 3;
    case USCRIPT_ARABIC: return 4;
    case USCRIPT_THAI: return 6;
    case USCRIPT_HANGUL: return 7;
    case USCRIPT_HIRAGANA:
    case USCRIPT_KATAKANA: return 8;
    case USCRIPT_HAN: return 9; // zh-CN (simplified Chinese)
    default: return 0;
  }
}

static const unsigned char kCRangeSpecificItemNum =
    sizeof(kUnicodeRangeToLangTable) / sizeof(kUnicodeRangeToLangTable[0]);

static unsigned char FindCharUnicodeRange(UChar32 ch) {
  UScriptCode script = uscript_getScript(ch, nullptr);
  return ScriptToRange(script);
}

static const char* GuessLangFromChar(UChar32 ch) {
  unsigned char unicode_range = FindCharUnicodeRange(ch);
  if (unicode_range < kCRangeSpecificItemNum)
    return kUnicodeRangeToLangTable[unicode_range];
  return 0;
}
#endif

// static
static AtomicString& MutableSystemFontFamily() {
  static base::NoDestructor<AtomicString> family;
  return *family;
}

// static
const AtomicString& FontCache::SystemFontFamily() {
  return MutableSystemFontFamily();
}

// static
void FontCache::SetSystemFontFamily(const AtomicString& family_name) {
  DCHECK(!family_name.empty());
  MutableSystemFontFamily() = family_name;
}

bool FontCache::GetFontForCharacter(UChar32 c,
                                    const char* preferred_locale,
                                    gfx::FallbackFontData* fallback_font) {
  if (Platform::Current()->GetSandboxSupport()) {
    return Platform::Current()
        ->GetSandboxSupport()
        ->GetFallbackFontForCharacter(c, preferred_locale, fallback_font);
  } else {
    std::string locale = preferred_locale ? preferred_locale : std::string();
    return gfx::GetFallbackFontForChar(c, locale, fallback_font);
  }
}

const SimpleFontData* FontCache::PlatformFallbackFontForCharacter(
    const FontDescription& font_description,
    UChar32 c,
    const SimpleFontData*,
    FontFallbackPriority fallback_priority) {
  if (IsEmojiPresentationEmoji(fallback_priority)) {
    // FIXME crbug.com/591346: We're overriding the fallback character here
    // with the FAMILY emoji in the hope to find a suitable emoji font.
    // This should be improved by supporting fallback for character
    // sequences like DIGIT ONE + COMBINING keycap etc.
    c = uchar::kFamily;
  }
#if BUILDFLAG(IS_NEVA_APPRUNTIME)
  // Guess language from character script
  if (!font_description.Locale() ||
      font_description.Locale()->LocaleString().empty()) {
    AtomicString locale(GuessLangFromChar(c));
    if (!locale.empty()) {
      FontDescription tmp_description(font_description);
      tmp_description.SetLocale(LayoutLocale::Get(locale));
      FontFaceCreationParams creation_params(
          tmp_description.Family().FamilyName());
      const FontPlatformData* platform_data =
          GetFontPlatformData(tmp_description, creation_params);
      if (platform_data && platform_data->FontContainsCharacter(c))
        return FontDataFromFontPlatformData(platform_data);

      // Set correspondence between locale and character to font cache so it
      // will retrieve correct font further on by the code.
      gfx::FallbackFontData tmp_fallback_font;
      FontCache::GetFontForCharacter(
          c, tmp_description.LocaleOrDefault().Ascii().data(),
          &tmp_fallback_font);
    }
  }
#endif

  // First try the specified font with standard style & weight.
  if (!IsEmojiPresentationEmoji(fallback_priority) &&
      (font_description.Style() == kItalicSlopeValue ||
       font_description.Weight() >= kBoldThreshold)) {
    const SimpleFontData* font_data =
        FallbackOnStandardFontStyle(font_description, c);
    if (font_data)
      return font_data;
  }

  gfx::FallbackFontData fallback_font;
  if (!FontCache::GetFontForCharacter(
          c,
          IsEmojiPresentationEmoji(fallback_priority)
              ? kColorEmojiLocale
              : font_description.LocaleOrDefault().Ascii().c_str(),
          &fallback_font)) {
    return nullptr;
  }

  FontFaceCreationParams creation_params;
  creation_params = FontFaceCreationParams(
// NOTE(neva): Change to blink::String from std::string on
// FontFaceCreationParams to avoid crash
#if defined(__GNUC__) && !defined(__clang__)
      blink::String(fallback_font.filepath.value()),
      fallback_font.fontconfig_interface_id,
#else
      fallback_font.filepath.value(), fallback_font.fontconfig_interface_id,
#endif
      fallback_font.ttc_index);

  // Changes weight and/or italic of given FontDescription depends on
  // the result of fontconfig so that keeping the correct font mapping
  // of the given character. See http://crbug.com/32109 for details.
  bool should_set_synthetic_bold = false;
  bool should_set_synthetic_italic = false;
  FontDescription description(font_description);
  if (fallback_font.is_bold && description.Weight() < kBoldThreshold) {
    description.SetWeight(kBoldWeightValue);
  }
  if (!fallback_font.is_bold && description.Weight() >= kBoldThreshold &&
      font_description.SyntheticBoldAllowed()) {
    should_set_synthetic_bold = true;
    description.SetWeight(kNormalWeightValue);
  }
  if (fallback_font.is_italic && description.Style() == kNormalSlopeValue) {
    description.SetStyle(kItalicSlopeValue);
  }
  if (!fallback_font.is_italic && (description.Style() == kItalicSlopeValue) &&
      font_description.SyntheticItalicAllowed()) {
    should_set_synthetic_italic = true;
    description.SetStyle(kNormalSlopeValue);
  }

  const FontPlatformData* substitute_platform_data =
      GetFontPlatformData(description, creation_params);
  if (!substitute_platform_data)
    return nullptr;

  FontPlatformData* platform_data =
      MakeGarbageCollected<FontPlatformData>(*substitute_platform_data);
  platform_data->SetSyntheticBold(should_set_synthetic_bold);
  platform_data->SetSyntheticItalic(should_set_synthetic_italic);
  return FontDataFromFontPlatformData(platform_data);
}

}  // namespace blink

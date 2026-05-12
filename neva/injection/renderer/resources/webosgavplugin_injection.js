// Copyright 2020 LG Electronics, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var webosgavplugin = new function() {
  // For plugin's call
  this.getMediaId = function() {
    return WebOSGAVInternal_.gavGetMediaId();
  };
  this.requestMediaLayer = function(mid, t) {
    return WebOSGAVInternal_.gavRequestMediaLayer(mid, t);
  };
  this.updateMediaLayerBounds = function(
      id, src_x, src_y, src_w, src_h, dst_x, dst_y, dst_w, dst_h) {
    return WebOSGAVInternal_.gavUpdateMediaLayerBounds(
        id, src_x, src_y, src_w, src_h, dst_x, dst_y, dst_w, dst_h);
  };
  this.destroyMediaLayer = function(id) {
    return WebOSGAVInternal_.gavDestroyMediaLayer(id);
  };
  this.updateMediaCropBounds = function(
      id, org_x, org_y, org_w, org_h, src_x, src_y, src_w, src_h, dst_x, dst_y,
      dst_w, dst_h) {
    return WebOSGAVInternal_.gavUpdateMediaCropBounds(
        id, org_x, org_y, org_w, org_h, src_x, src_y, src_w, src_h, dst_x,
        dst_y, dst_w, dst_h);
  };
  this.setMediaProperty = function(id, name, value) {
    return WebOSGAVInternal_.gavSetMediaProperty(id, name, value);
  };
  // For WAM's callback on 'this.gavRequestMediaLayer'
  function onCML(objs, mid, lid, t) {
    for (var i = 0; i < objs.length; ++i) {
      var obj = objs[i];
      if (obj.gavMediaId == mid &&
          typeof (obj.gavOnCreatedMediaLayer) != 'undefined') {
        obj.gavOnCreatedMediaLayer(lid, t);
        return true;
      }
    }
    return false;
  };

  function onCMLAll(doc, mid, lid, t) {
    if (onCML(doc.getElementsByTagName('object'), mid, lid, t) ||
        onCML(doc.getElementsByTagName('embed'), mid, lid, t))
      return true;
    var iframes = doc.getElementsByTagName('iframe');
    for (var i = 0; i < iframes.length; ++i) {
      if (onCMLAll(iframes[i].contentWindow.document, mid, lid, t))
        return true;
    }
    return false;
  }
  this.onCreatedMediaLayer = function(mid, lid, t) {
    onCMLAll(document, mid, lid, t) ||
        console.error(
            '[webosgavplugin] onCreatedMediaLayer: No mediaId:' + mid);
  };
  // For WAM's event call for willDestroyMediaLayer
  function onWDML(objs, mid) {
    for (var i = 0; i < objs.length; ++i) {
      var obj = objs[i];
      if (obj.gavMediaId == mid &&
          typeof (obj.gavWillDestroyMediaLayer) != 'undefined') {
        obj.gavWillDestroyMediaLayer();
        return true;
      }
    }
    return false;
  }

  function onWDMLAll(doc, mid) {
    if (onWDML(doc.getElementsByTagName('object'), mid) ||
        onWDML(doc.getElementsByTagName('embed'), mid))
      return true;
    var iframes = doc.getElementsByTagName('iframe');
    for (var i = 0; i < iframes.length; ++i) {
      if (onWDMLAll(iframes[i].contentWindow.document, mid))
        return true;
    }
    return false;
  }
  this.willDestroyMediaLayer = function(mid) {
    onWDMLAll(document, mid) ||
        console.error(
            '[webosgavplugin] willDestroyMediaLayer: No mediaId:' + mid);
  };
};

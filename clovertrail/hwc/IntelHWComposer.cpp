/*
 * Copyright © 2012 Intel Corporation
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Jackie Li <yaodong.li@intel.com>
 *
 */
#include <cutils/log.h>
#include <cutils/atomic.h>

#include <IntelHWComposer.h>
#include <IntelOverlayUtil.h>
#include <IntelWidiPlane.h>

//#define INTEL_EXT_SF_NEED_SWAPBUFFER
#define INTEL_EXT_SF_ANIMATION_HINT

#include <IntelHWComposerCfg.h>

IntelHWComposer::~IntelHWComposer()
{
    LOGD_IF(ALLOW_HWC_PRINT, "%s\n", __func__);

    delete mLayerList;
    delete mPlaneManager;
    delete mBufferManager;
    delete mDrm;

    // stop uevent observer
    stopObserver();
}

bool IntelHWComposer::overlayPrepare(int index, hwc_layer_t *layer, int flags)
{
    if (!layer) {
        LOGE("%s: Invalid layer\n", __func__);
        return false;
    }

    // allocate overlay plane
    IntelDisplayPlane *plane = mPlaneManager->getOverlayPlane();
    if (!plane) {
        LOGE("%s: failed to create overlay plane\n", __func__);
        return false;
    }

    int dstLeft = layer->displayFrame.left;
    int dstTop = layer->displayFrame.top;
    int dstRight = layer->displayFrame.right;
    int dstBottom = layer->displayFrame.bottom;

    // setup plane parameters
    plane->setPosition(dstLeft, dstTop, dstRight, dstBottom);

    plane->setPipeByMode(mDrm->getDisplayMode());

    // attach plane to hwc layer
    mLayerList->attachPlane(index, plane, flags);

    return true;
}

bool IntelHWComposer::spritePrepare(int index, hwc_layer_t *layer, int flags)
{
    if (!layer) {
        LOGE("%s: Invalid layer\n", __func__);
        return false;
    }

    // allocate sprite plane
    IntelDisplayPlane *plane = mPlaneManager->getSpritePlane();
    if (!plane) {
        LOGE("%s: failed to create sprite plane\n", __func__);
        return false;
    }

    int dstLeft = layer->displayFrame.left;
    int dstTop = layer->displayFrame.top;
    int dstRight = layer->displayFrame.right;
    int dstBottom = layer->displayFrame.bottom;

    // setup plane parameters
    plane->setPosition(dstLeft, dstTop, dstRight, dstBottom);

    // attach plane to hwc layer
    mLayerList->attachPlane(index, plane, flags);

    return true;
}

bool IntelHWComposer::primaryPrepare(int index, hwc_layer_t *layer, int flags)
{
    if (!layer) {
        LOGE("%s: Invalid layer\n", __func__);
        return false;
    }
    int dstLeft = layer->displayFrame.left;
    int dstTop = layer->displayFrame.top;
    int dstRight = layer->displayFrame.right;
    int dstBottom = layer->displayFrame.bottom;

    // allocate sprite plane
    IntelDisplayPlane *plane = mPlaneManager->getPrimaryPlane(0);
    if (!plane) {
        LOGE("%s: failed to create sprite plane\n", __func__);
        return false;
    }

    // TODO: check external display status, and attach plane

    // setup plane parameters
    plane->setPosition(dstLeft, dstTop, dstRight, dstBottom);

    // attach plane to hwc layer
    mLayerList->attachPlane(index, plane, flags);

    return true;
}

bool IntelHWComposer::isForceOverlay(hwc_layer_t *layer)
{
    if (!layer)
        return false;

    intel_gralloc_buffer_handle_t *grallocHandle =
        (intel_gralloc_buffer_handle_t*)layer->handle;

    if (!grallocHandle)
        return false;

    if (grallocHandle->format != HAL_PIXEL_FORMAT_INTEL_HWC_NV12)
        return false;

    // map payload buffer
    IntelPayloadBuffer buffer(mGrallocBufferManager, grallocHandle->fd[GRALLOC_SUB_BUFFER1]);

    intel_gralloc_payload_t *payload =
        (intel_gralloc_payload_t*)buffer.getCpuAddr();
    if (!payload) {
        LOGE("%s: invalid address\n", __func__);
        return false;
    }

    bool ret = (payload->force_output_method == OUTPUT_FORCE_OVERLAY) ? true : false;
    return ret;
}

// TODO: re-implement this function after video interface
// is ready.
// Currently, LayerTS::setGeometry will set compositionType
// to HWC_OVERLAY. HWC will change it to HWC_FRAMEBUFFER
// if HWC found this layer was NOT a overlay layer (can NOT
// be handled by hardware overlay)
bool IntelHWComposer::isOverlayLayer(hwc_layer_list_t *list,
                                     int index,
                                     hwc_layer_t *layer,
                                     int& flags)
{
    bool needClearFb = false;
    bool forceOverlay = false;
    bool useOverlay = false;

    if (!list || !layer)
        return false;

    intel_gralloc_buffer_handle_t *grallocHandle =
        (intel_gralloc_buffer_handle_t*)layer->handle;

    if (!grallocHandle)
        return false;

    IntelWidiPlane* widiPlane = (IntelWidiPlane*)mPlaneManager->getWidiPlane();

    // clear hints
    layer->hints = 0;

    // check format
    if (mLayerList->getLayerType(index) != IntelHWComposerLayer::LAYER_TYPE_YUV) {
        useOverlay = false;
        goto out_check;
    }

    // Got a YUV layer, check external display status for extend video mode
    if (widiPlane->isActive()) {

        int srcWidth = layer->sourceCrop.right - layer->sourceCrop.left;
        int srcHeight = layer->sourceCrop.bottom - layer->sourceCrop.top;

        if(!(widiPlane->isExtVideoAllowed()) || (srcWidth < 176 || srcHeight < 144)
            || (grallocHandle->format != HAL_PIXEL_FORMAT_INTEL_HWC_NV12)) {
           /* if extended video mode is not allowed or the resolution of video less than
            * QCIF (176 x 144) or Software decoder (e.g. VP8) is used, we stop here and let
            * the video to be rendered via GFx plane by surface flinger. Video encoder has
            * limitation that HW encoder can't encode video that is less than QCIF
            */
            useOverlay = false;
            goto out_check;
        }
        if(widiPlane->isPlayerOn() && widiPlane->isExtVideoAllowed()) {
            LOGD_IF(ALLOW_HWC_PRINT, "isOverlayLayer: widi video on and force overlay");
            forceOverlay = true;
        }
    }

    // force to use overlay in video extend mode
    if (mDrm->getDisplayMode() == OVERLAY_EXTEND)
        forceOverlay = true;

    // check buffer usage
    if ((grallocHandle->usage & GRALLOC_USAGE_PROTECTED) || isForceOverlay(layer)) {
        LOGD_IF(ALLOW_HWC_PRINT, "isOverlayLayer: protected video/force Overlay");
        forceOverlay = true;
    }

    // check blending, overlay cannot support blending
    if (layer->blending != HWC_BLENDING_NONE) {
        useOverlay = false;
        goto out_check;
    }

    // fall back if HWC_SKIP_LAYER was set, if forced to use
    // overlay skip this check
    if (!forceOverlay && (layer->flags & HWC_SKIP_LAYER)) {
        LOGD_IF(ALLOW_HWC_PRINT, "isOverlayLayer: skip layer was set");
        useOverlay = false;
        goto out_check;
   }

    // check visible regions
    if (layer->visibleRegionScreen.numRects > 1) {
        useOverlay = false;
        goto out_check;
    }

    // TODO: not support OVERLAY_CLONE_MIPI0
    if (mDrm->getDisplayMode() == OVERLAY_CLONE_MIPI0) {
        useOverlay = false;
        goto out_check;
    }

    // fall back if YUV Layer is in the middle of
    // other layers and covers the layers under it.
    if (!forceOverlay) {
        for (int i = index - 1; i >= 0; i--) {
            if (areLayersIntersecting(layer, &list->hwLayers[i])) {
                useOverlay = false;
                goto out_check;
            }
        }
    }

    // check whether layer are covered by layers above it
    // if layer is covered by a layer which needs blending,
    // clear corresponding region in frame buffer
    for (size_t i = index + 1; i < list->numHwLayers; i++) {
        if (areLayersIntersecting(&list->hwLayers[i], layer)) {
            LOGD_IF(ALLOW_HWC_PRINT,
                "%s: overlay %d is covered by layer %d\n", __func__, index, i);
                if (list->hwLayers[i].blending !=  HWC_BLENDING_NONE)
                    mLayerList->setNeedClearup(index, true);
        }
    }

    useOverlay = true;
    needClearFb = true;
out_check:
    if (forceOverlay) {
        // clear HWC_SKIP_LAYER flag so that force to use overlay
        LOGD("isOverlayLayer: force to use overlay");
        layer->flags &= ~HWC_SKIP_LAYER;
        mLayerList->setForceOverlay(index, true);
        layer->compositionType = HWC_OVERLAY;
        useOverlay = true;
        needClearFb = true;
#ifdef INTEL_EXT_SF_ANIMATION_HINT
        layer->hints |= HWC_HINT_DISABLE_ANIMATION;
#endif
    }

    // check if frame buffer clear is needed
    if (useOverlay) {
        LOGD("isOverlayLayer: got an overlay layer");
        if (needClearFb) {
            //layer->hints |= HWC_HINT_CLEAR_FB;
            LOGD_IF(ALLOW_HWC_PRINT, "isOverlayLayer: clear fb");
            mForceSwapBuffer = true;
        }
        layer->compositionType = HWC_OVERLAY;
    }

    flags = 0;
    return useOverlay;
}

// isSpriteLayer: check whether a given @layer can be handled
// by a hardware sprite plane.
// A layer is a sprite layer when
// 1) layer is RGB layer &&
// 2) No active external display (TODO: support external display)
// 3) HWC_SKIP_LAYER flag wasn't set by surface flinger
// 4) layer requires no blending or premultipled blending
// 5) layer has no transform (rotation, scaling)
bool IntelHWComposer::isSpriteLayer(hwc_layer_list_t *list,
                                    int index,
                                    hwc_layer_t *layer,
                                    int& flags)
{
    bool needClearFb = false;
    bool forceSprite = false;
    bool useSprite = false;

    int srcWidth, srcHeight;
    int dstWidth, dstHeight;

    if (!list || !layer)
        return false;

    intel_gralloc_buffer_handle_t *grallocHandle =
        (intel_gralloc_buffer_handle_t*)layer->handle;

    if (!grallocHandle) {
        LOGD_IF(ALLOW_HWC_PRINT, "%s: invalid gralloc handle\n", __func__);
        return false;
    }

    // check whether pixel format is supported RGB formats
    if (mLayerList->getLayerType(index) != IntelHWComposerLayer::LAYER_TYPE_RGB) {
        LOGD_IF(ALLOW_HWC_PRINT,
                "%s: invalid format 0x%x\n", __func__, grallocHandle->format);
        useSprite = false;
        goto out_check;
    }

    // Got a RGB layer, disable sprite plane when Widi is active
    if (mPlaneManager->isWidiActive()) {
        useSprite = false;
        goto out_check;
    }

    // disable sprite plane when HDMI is connected
    // FIXME: add HDMI sprite support later
    if (mDrm->getOutputConnection(OUTPUT_HDMI) == DRM_MODE_CONNECTED) {
        useSprite = false;
        goto out_check;
    }

    // fall back if HWC_SKIP_LAYER was set
    if ((layer->flags & HWC_SKIP_LAYER)) {
        LOGD_IF(ALLOW_HWC_PRINT, "isSpriteLayer: HWC_SKIP_LAYER");
        useSprite = false;
        goto out_check;
    }

    // check usage???

    // check blending, only support none & premultipled blending
    // clear frame buffer region if layer has no blending
    if (layer->blending != HWC_BLENDING_PREMULT &&
        layer->blending != HWC_BLENDING_NONE) {
        LOGD("isSpriteLayer: unsupported blending");
        useSprite = false;
        goto out_check;
    }

    // check rotation
    if (layer->transform) {
        LOGD_IF(ALLOW_HWC_PRINT, "isSpriteLayer: need do transform");
        useSprite = false;
        goto out_check;
    }

     // check scaling
    srcWidth = layer->sourceCrop.right - layer->sourceCrop.left;
    srcHeight = layer->sourceCrop.bottom - layer->sourceCrop.top;
    dstWidth = layer->displayFrame.right - layer->displayFrame.left;
    dstHeight = layer->displayFrame.bottom - layer->displayFrame.top;

    if ((srcWidth == dstWidth) && (srcHeight == dstHeight))
        useSprite = true;
    else
        LOGD_IF(ALLOW_HWC_PRINT,
               "isSpriteLayer: src W,H [%d, %d], dst W,H [%d, %d]",
               srcWidth, srcHeight, dstWidth, dstHeight);

    if (layer->blending == HWC_BLENDING_NONE)
        needClearFb = true;
out_check:
    if (forceSprite) {
        // clear HWC_SKIP_LAYER flag so that force to use overlay
        LOGD("isSpriteLayer: force to use sprite");
        layer->flags &= ~HWC_SKIP_LAYER;
        mLayerList->setForceOverlay(index, true);
        layer->compositionType = HWC_OVERLAY;
        useSprite = true;
    }

    // check if frame buffer clear is needed
    if (useSprite) {
        LOGD("isSpriteLayer: got a sprite layer");
        if (needClearFb) {
            LOGD_IF(ALLOW_HWC_PRINT, "isSpriteLayer: clear fb");
            //layer->hints |= HWC_HINT_CLEAR_FB;
            mForceSwapBuffer = true;
        }
        layer->compositionType = HWC_OVERLAY;
    }

    flags = 0;
    return useSprite;
}

// isPrimaryLayer: check whether we can use primary plane to handle
// the given @layer.
// primary plane can be used only when
// 1) @layer is on the top of other layers (FIXME: not necessary, remove it
//    after introducing z order configuration)
// 2) all other layers were handled by HWC.
// 3) @layer is a sprite layer
// 4) @layer wasn't handled by sprite
bool IntelHWComposer::isPrimaryLayer(hwc_layer_list_t *list,
                                     int index,
                                     hwc_layer_t *layer,
                                     int& flags)
{
#ifndef INTEL_RGB_OVERLAY
    // only use primary when layer is the top layer
    if ((size_t)index != (list->numHwLayers - 1))
        return false;
#endif


    // if a layer has already been handled, further check if it's a
    // sprite layer/overlay layer, if so, we simply bypass this layer.
    if (layer->compositionType == HWC_OVERLAY) {
        IntelDisplayPlane *plane = mLayerList->getPlane(index);
        if (plane) {
            switch (plane->getPlaneType()) {
            case IntelDisplayPlane::DISPLAY_PLANE_PRIMARY:
                // detach plane & re-check it
                mLayerList->detachPlane(index, plane);
                layer->compositionType = HWC_FRAMEBUFFER;
                layer->hints = 0;
                break;
            case IntelDisplayPlane::DISPLAY_PLANE_OVERLAY:
            case IntelDisplayPlane::DISPLAY_PLANE_SPRITE:
            default:
                return false;
            }
        }
    }

    // check whether all other layers were handled by HWC
    for (size_t i = 0; i < list->numHwLayers; i++) {
        if ((list->hwLayers[i].compositionType != HWC_OVERLAY) && (i != index))
            return false;
    }

    return isSpriteLayer(list, index, layer, flags);
}

void IntelHWComposer::revisitLayerList(hwc_layer_list_t *list, bool isGeometryChanged)
{
    int zOrderConfig = IntelDisplayPlaneManager::ZORDER_OcOaP;

    if (!list)
        return;

    for (size_t i = 0; i < list->numHwLayers; i++) {
        int flags = 0;

        // also need check whether a layer can be handled in general
        if (!isHWCLayer(&list->hwLayers[i]))
            continue;

        // make sure all protected layers were marked as overlay
        if (mLayerList->isProtectedLayer(i))
            list->hwLayers[i].compositionType = HWC_OVERLAY;
        // check if we can apply primary plane to an RGB layer
        // if overlay plane was used for a YUV layer, force overlay layer to
        // be the bottom layer.
        if (mLayerList->getLayerType(i) != IntelHWComposerLayer::LAYER_TYPE_RGB) {
            zOrderConfig = IntelDisplayPlaneManager::ZORDER_POcOa;
            continue;
        }

        if (isPrimaryLayer(list, i, &list->hwLayers[i], flags)) {
            bool ret = primaryPrepare(i, &list->hwLayers[i], flags);
            if (!ret) {
                LOGE("%s: failed to prepare primary\n", __func__);
                list->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
                list->hwLayers[i].hints = 0;
            }
        }
    }

    if (isGeometryChanged)
        mPlaneManager->setZOrderConfig(zOrderConfig, 0);

}

void IntelHWComposer::dumpLayerList(hwc_layer_list_t *list)
{
    if (!list)
        return;

    LOGD("\n");
    LOGD("DUMP LAYER LIST START");
    LOGD("num of layers: %d", list->numHwLayers);
    for (size_t i = 0; i < list->numHwLayers; i++) {
	int dstLeft = list->hwLayers[i].displayFrame.left;
	int dstTop = list->hwLayers[i].displayFrame.top;
	int dstRight = list->hwLayers[i].displayFrame.right;
	int dstBottom = list->hwLayers[i].displayFrame.bottom;

        LOGD("Layer type: %s",
        (mLayerList->getLayerType(i) != IntelHWComposerLayer::LAYER_TYPE_YUV) ?
        "RGB" : "YUV");
        LOGD("Layer blending: 0x%x", list->hwLayers[i].blending);
        LOGD("Layer positon: (%d, %d) - (%dx%d)", dstLeft, dstTop,
             dstRight - dstLeft, dstBottom - dstTop);
        LOGD("Layer transform: 0x%x\n", list->hwLayers[i].transform);
        LOGD("Layer handle: 0x%x\n", (uint32_t)list->hwLayers[i].handle);
        LOGD("Layer flags: 0x%x\n", list->hwLayers[i].flags);
        LOGD("\n");
    }
    LOGD("DUMP LAYER LIST END");
    LOGD("\n");
}

bool IntelHWComposer::isScreenshotActive(hwc_layer_list_t *list)
{
    if (!list || !list->numHwLayers)
        return false;

    hwc_layer_t *topLayer = &list->hwLayers[list->numHwLayers - 1];
    IntelWidiPlane* widiPlane = (IntelWidiPlane*)mPlaneManager->getWidiPlane();

    if (mDrm->getDisplayMode() == OVERLAY_EXTEND)
        return false;

    if (widiPlane->isActive())
        return false;

    if (!topLayer) {
        LOGW("This might be a surfaceflinger BUG\n");
        return false;
    }

    if (!(topLayer->flags & HWC_SKIP_LAYER))
        return false;

    for (size_t i = 0; i < list->numHwLayers; i++) {
        if (mLayerList->isProtectedLayer(i))
            return false;
    }

    int x = topLayer->displayFrame.left;
    int y = topLayer->displayFrame.top;
    int w = topLayer->displayFrame.right - topLayer->displayFrame.left;
    int h = topLayer->displayFrame.bottom - topLayer->displayFrame.top;

    drmModeFBPtr fbInfo =
        IntelHWComposerDrm::getInstance().getOutputFBInfo(OUTPUT_MIPI0);

    if (x == 0 && y == 0 && w == fbInfo->width && h == fbInfo->height)
        return true;

    return false;
}

// When the geometry changed, we need
// 0) reclaim all allocated planes, reclaimed planes will be disabled
//    on the start of next frame. A little bit tricky, we cannot disable the
//    planes right after geometry is changed since there's no data in FB now,
//    so we need to wait FB is update then disable these planes.
// 1) build a new layer list for the changed hwc_layer_list
// 2) attach planes to these layers which can be handled by HWC
void IntelHWComposer::onGeometryChanged(hwc_layer_list_t *list)
{
    bool firstTime = true;

    // reclaim all planes
    bool ret = mLayerList->invalidatePlanes();
    if (!ret) {
        LOGE("%s: failed to reclaim allocated planes\n", __func__);
        return;
    }

    // update layer list with new list
    mLayerList->updateLayerList(list);

    // TODO: uncomment it to print out layer list info
    // dumpLayerList(list);

    if (isScreenshotActive(list)) {
        LOGD_IF(ALLOW_HWC_PRINT, "%s: Screenshot Active!\n", __func__);
        goto out_check;
    }

    for (size_t i = 0; list && i < list->numHwLayers; i++) {
        // check whether a layer can be handled in general
        if (!isHWCLayer(&list->hwLayers[i]))
            continue;

        // further check whether a layer can be handle by overlay/sprite
        int flags = 0;
        bool hasOverlay = mPlaneManager->hasFreeOverlays();
        bool hasSprite = mPlaneManager->hasFreeSprites();

        if (hasOverlay && isOverlayLayer(list, i, &list->hwLayers[i], flags)) {
            ret = overlayPrepare(i, &list->hwLayers[i], flags);
            if (!ret) {
                LOGE("%s: failed to prepare overlay\n", __func__);
                list->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
                list->hwLayers[i].hints = 0;
            }
        } else if (hasSprite && isSpriteLayer(list, i, &list->hwLayers[i], flags)) {
            ret = spritePrepare(i, &list->hwLayers[i], flags);
            if (!ret) {
                LOGE("%s: failed to prepare sprite\n", __func__);
                list->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
                list->hwLayers[i].hints = 0;
            }
        } else {
            list->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
            if(firstTime && mPlaneManager->isWidiActive() ) {
                IntelWidiPlane *p = (IntelWidiPlane *)mPlaneManager->getWidiPlane();
                p->setOrientation(list->hwLayers[i].transform);
                firstTime = false;
            }
        }
    }

out_check:
    // revisit each layer, make sure protected layers were handled by hwc,
    // and check if we can make use of primary plane
    revisitLayerList(list, true);

    // disable reclaimed planes
    mPlaneManager->disableReclaimedPlanes(IntelDisplayPlane::DISPLAY_PLANE_SPRITE);
    mPlaneManager->disableReclaimedPlanes(IntelDisplayPlane::DISPLAY_PLANE_PRIMARY);
}

// This function performs:
// 1) update layer's transform to data buffer's payload buffer, so that video
//    driver can get the latest transform info of this layer
// 2) if rotation is needed, video driver would setup the rotated buffer, then
//    update buffer's payload to inform HWC rotation buffer is available.
// 3) HWC would keep using ST till all rotation buffers are ready.
// Return: false if HWC is NOT ready to switch to overlay, otherwise true.
bool IntelHWComposer::useOverlayRotation(hwc_layer_t *layer,
                                         int index,
                                         uint32_t& handle,
                                         int& w, int& h,
                                         int& srcX, int& srcY,
                                         int& srcW, int& srcH,
                                         uint32_t &transform)
{
    bool useOverlay = false;
    uint32_t hwcLayerTransform;
    IntelWidiPlane* widiplane = (IntelWidiPlane*)mPlaneManager->getWidiPlane();

    // FIXME: workaround for rotation issue, remove it later
    static int counter = 0;
    uint32_t metadata_transform = 0;
    uint32_t displayMode = 0;

    if (!layer)
        return false;

    intel_gralloc_buffer_handle_t *grallocHandle =
        (intel_gralloc_buffer_handle_t*)layer->handle;

    if (!grallocHandle)
        return false;

    // detect video mode change
    displayMode = mDrm->getDisplayMode();
    if(widiplane->isStreaming())
        displayMode = OVERLAY_EXTEND;

    if (grallocHandle->format == HAL_PIXEL_FORMAT_INTEL_HWC_NV12) {
        // map payload buffer
        IntelPayloadBuffer buffer(mGrallocBufferManager, grallocHandle->fd[GRALLOC_SUB_BUFFER1]);
        intel_gralloc_payload_t *payload = (intel_gralloc_payload_t*)buffer.getCpuAddr();
        if (!payload) {
            LOGE("%s: invalid address\n", __func__);
            return false;
        }

        if (payload->force_output_method == OUTPUT_FORCE_GPU) {
            LOGD_IF(ALLOW_HWC_PRINT,
                    "%s: force to use surface texture.", __func__);
            return false;
        }

        metadata_transform = payload->metadata_transform;

        //For extend mode, we ignore WM rotate info
        if (displayMode == OVERLAY_EXTEND) {
            transform = metadata_transform;
        }

        if (!transform) {
            LOGD_IF(ALLOW_HWC_PRINT,
                    "%s: use overlay to display original buffer.", __func__);
            return true;
        }

        if (transform != payload->client_transform) {
            LOGV("%s: rotation buffer was not prepared by client!\n", __func__);
            return false;
        }

        // update handle, w & h to rotation buffer
        handle = payload->rotated_buffer_handle;
        w = payload->rotated_width;
        h = payload->rotated_height;
        //wait video rotated buffer idle
        mGrallocBufferManager->waitIdle(handle);
        // NOTE: exchange the srcWidth & srcHeight since
        // video driver currently doesn't call native_window_*
        // helper functions to update info for rotation buffer.
        if (transform == HAL_TRANSFORM_ROT_90 ||
                transform == HAL_TRANSFORM_ROT_270) {
            int temp = srcH;
            srcH = srcW;
            srcW = temp;
            temp = srcX;
            srcX = srcY;
            srcY = temp;

        }

        // skip pading bytes in rotate buffer
        switch(transform) {
            case HAL_TRANSFORM_ROT_90:
                srcX += ((srcW + 0xf) & ~0xf) - srcW;
                break;
            case HAL_TRANSFORM_ROT_180:
                srcX += ((srcW + 0xf) & ~0xf) - srcW;
                srcY += ((srcH + 0xf) & ~0xf) - srcH;
                break;
            case HAL_TRANSFORM_ROT_270:
                srcY += ((srcH + 0xf) & ~0xf) - srcH;
                break;
            default:
                break;
        }
    } else {
        //For software codec, overlay can't handle rotate
        //and fallback to surface texture.
        if ((displayMode != OVERLAY_EXTEND) && transform) {
            LOGD_IF(ALLOW_HWC_PRINT,
                    "%s: software codec rotation, back to ST!", __func__);
            return false;
        }
        //set this flag for mapping correct buffer
        transform = 0;
    }

    //for most of cases, we handle rotate by overlay
    useOverlay = true;
    return useOverlay;
}

// This function performs:
// Acquire bool BobDeinterlace from video driver
// If ture, use bob deinterlace, otherwise, use weave deinterlace
bool IntelHWComposer::isBobDeinterlace(hwc_layer_t *layer)
{
    bool bobDeinterlace = false;

    if (!layer)
        return bobDeinterlace;

    intel_gralloc_buffer_handle_t *grallocHandle =
        (intel_gralloc_buffer_handle_t*)layer->handle;

    if (!grallocHandle) {
        return bobDeinterlace;
    }

    if (grallocHandle->format != HAL_PIXEL_FORMAT_INTEL_HWC_NV12)
        return bobDeinterlace;

    // map payload buffer
    IntelPayloadBuffer buffer(mGrallocBufferManager, grallocHandle->fd[GRALLOC_SUB_BUFFER1]);

    intel_gralloc_payload_t *payload = (intel_gralloc_payload_t*)buffer.getCpuAddr();
    if (!payload) {
        LOGE("%s: invalid address\n", __func__);
        return bobDeinterlace;
    }

    bobDeinterlace = (payload->bob_deinterlace == 1) ? true : false;
    return bobDeinterlace;
}

// when buffer handle is changed, we need
// 0) get plane's data buffer if a plane was attached to a layer
// 1) update plane's data buffer with the new buffer handle
// 2) set the updated data buffer back to plane
bool IntelHWComposer::updateLayersData(hwc_layer_list_t *list)
{
    IntelDisplayPlane *plane = 0;
    IntelWidiPlane* widiplane = NULL;
    bool ret = true;
    bool handled = true;

    if (mPlaneManager->isWidiActive()) {
         widiplane = (IntelWidiPlane*) mPlaneManager->getWidiPlane();
         mFBDev->bBypassPost = 0;
    } else
         mFBDev->bBypassPost = cfg.bypasspost;

    for (size_t i=0 ; i<list->numHwLayers ; i++) {
        hwc_layer_t *layer = &list->hwLayers[i];
        intel_gralloc_buffer_handle_t *grallocHandle =
            (intel_gralloc_buffer_handle_t*)layer->handle;

        if (grallocHandle &&
            grallocHandle->format == HAL_PIXEL_FORMAT_INTEL_HWC_NV12) {
            // map payload buffer
            IntelPayloadBuffer buffer(mGrallocBufferManager, grallocHandle->fd[GRALLOC_SUB_BUFFER1]);

            intel_gralloc_payload_t *payload = (intel_gralloc_payload_t*)buffer.getCpuAddr();

            if (!payload) {
                LOGE("%s: invalid address\n", __func__);
                return false;
            }
            //wait video buffer idle
            mGrallocBufferManager->waitIdle(payload->khandle);
        }
        plane = mLayerList->getPlane(i);
        if (!plane)
            continue;

        // clear layer's visible region if need clear up flag was set
        // and sprite plane was used as primary plane (point to FB)
        if (mLayerList->getNeedClearup(i) &&
            mPlaneManager->primaryAvailable(0)) {
            LOGD_IF(ALLOW_HWC_PRINT,
                  "updateLayersData: clear visible region of layer %d", i);
            list->hwLayers[i].hints |= HWC_HINT_CLEAR_FB;
        }

        int bobDeinterlace;
        int srcX = layer->sourceCrop.left;
        int srcY = layer->sourceCrop.top;
        int srcWidth = layer->sourceCrop.right - layer->sourceCrop.left;
        int srcHeight = layer->sourceCrop.bottom - layer->sourceCrop.top;
        int planeType = plane->getPlaneType();

        if(srcHeight == 1 || srcWidth == 1) {
            mLayerList->detachPlane(i, plane);
            layer->compositionType = HWC_FRAMEBUFFER;
            handled = false;
            continue;
        }

        // get & setup overlay data buffer
        IntelDisplayBuffer *buffer = plane->getDataBuffer();
        IntelDisplayDataBuffer *dataBuffer =
            reinterpret_cast<IntelDisplayDataBuffer*>(buffer);
        if (!dataBuffer) {
            LOGE("%s: invalid overlay data buffer\n", __func__);
            continue;
        }

        // if invalid gralloc buffer handle, throw back this layer to SF
        if (!grallocHandle) {
                LOGE("%s: invalid buffer handle\n", __func__);
                mLayerList->detachPlane(i, plane);
                layer->compositionType = HWC_FRAMEBUFFER;
                handled = false;
                continue;
        }

        int bufferWidth = grallocHandle->width;
        int bufferHeight = grallocHandle->height;
        uint32_t bufferHandle = grallocHandle->fd[GRALLOC_SUB_BUFFER0];
        int format = grallocHandle->format;
        uint32_t transform = layer->transform;

        if (planeType == IntelDisplayPlane::DISPLAY_PLANE_OVERLAY) {
            if (widiplane) {
               widiplane->setOverlayData(grallocHandle, srcWidth, srcHeight);
               continue;
            }

            IntelOverlayContext *overlayContext =
                reinterpret_cast<IntelOverlayContext*>(plane->getContext());
            int flags = mLayerList->getFlags(i);

            // disable overlay if DELAY_DISABLE flag was set
            if (flags & IntelDisplayPlane::DELAY_DISABLE) {
                LOGD_IF(ALLOW_HWC_PRINT,
                       "updateLayerData: disable plane (DELAY)!");
                flags &= ~IntelDisplayPlane::DELAY_DISABLE;
                mLayerList->setFlags(i, flags);
                plane->disable();
            }

            // check if can switch to overlay
            bool useOverlay = useOverlayRotation(layer, i,
                                                 bufferHandle,
                                                 bufferWidth,
                                                 bufferHeight,
                                                 srcX,
                                                 srcY,
                                                 srcWidth,
                                                 srcHeight,
                                                 transform);

            if (!useOverlay) {
                LOGD_IF(ALLOW_HWC_PRINT,
                       "updateLayerData: useOverlayRotation failed!");
                if (!mLayerList->getForceOverlay(i)) {
                    LOGD_IF(ALLOW_HWC_PRINT,
                           "updateLayerData: fallback to ST to do rendering!");
                    // fallback to ST to render this frame
                    layer->compositionType = HWC_FRAMEBUFFER;
                    mForceSwapBuffer = true;
                    handled = false;
                }
                // disable overlay when rotated buffer is not ready
                flags |= IntelDisplayPlane::DELAY_DISABLE;
                mLayerList->setFlags(i, flags);
                continue;
            }

            bobDeinterlace = isBobDeinterlace(layer);
            if (bobDeinterlace) {
                flags |= IntelDisplayPlane::BOB_DEINTERLACE;
            } else {
                flags &= ~IntelDisplayPlane::BOB_DEINTERLACE;
            }
            mLayerList->setFlags(i, flags);

            // clear FB first on first overlay frame
            if (layer->compositionType == HWC_FRAMEBUFFER) {
                LOGD_IF(ALLOW_HWC_PRINT,
                       "updateLayerData: first overlay frame clear fb");
                //layer->hints |= HWC_HINT_CLEAR_FB;
                mForceSwapBuffer = true;
            }

            // switch to overlay
            layer->compositionType = HWC_OVERLAY;

            // gralloc buffer is not aligned to 32 pixels
            uint32_t grallocStride = align_to(bufferWidth, 32);
            int format = grallocHandle->format;

            dataBuffer->setFormat(format);
            dataBuffer->setStride(grallocStride);
            dataBuffer->setWidth(bufferWidth);
            dataBuffer->setHeight(bufferHeight);
            dataBuffer->setCrop(srcX, srcY, srcWidth, srcHeight);
            dataBuffer->setDeinterlaceType(bobDeinterlace);
            // set the data buffer back to plane
            ret = ((IntelOverlayPlane*)plane)->setDataBuffer(bufferHandle,
                                                             transform,
                                                             grallocHandle);
            if (!ret) {
                LOGE("%s: failed to update overlay data buffer\n", __func__);
                mLayerList->detachPlane(i, plane);
                layer->compositionType = HWC_FRAMEBUFFER;
                handled = false;
            }
        } else if (planeType == IntelDisplayPlane::DISPLAY_PLANE_SPRITE ||
                   planeType == IntelDisplayPlane::DISPLAY_PLANE_PRIMARY) {

            // adjust the buffer format if no blending is needed
            // some test cases would fail due to a weird format!
            if (layer->blending == HWC_BLENDING_NONE) {
                switch (format) {
                    case HAL_PIXEL_FORMAT_BGRA_8888:
                    format = HAL_PIXEL_FORMAT_BGRX_8888;
                    break;
                case HAL_PIXEL_FORMAT_RGBA_8888:
                    format = HAL_PIXEL_FORMAT_RGBX_8888;
                    break;
                }
            }

            // set data buffer format
            dataBuffer->setFormat(format);
            dataBuffer->setWidth(bufferWidth);
            dataBuffer->setHeight(bufferHeight);
            dataBuffer->setCrop(srcX, srcY, srcWidth, srcHeight);
            // set the data buffer back to plane
            ret = plane->setDataBuffer(bufferHandle, transform, grallocHandle);
            if (!ret) {
                LOGE("%s: failed to update sprite data buffer\n", __func__);
                mLayerList->detachPlane(i, plane);
                layer->compositionType = HWC_FRAMEBUFFER;
                handled = false;
            }
        } else {
            LOGW("%s: invalid plane type %d\n", __func__, planeType);
            continue;
        }
    }

    return handled;
}

// Check the usage of a buffer
// only following usages were accepted:
// TODO: list valid usages
bool IntelHWComposer::isHWCUsage(int usage)
{
    // For SW access buffer, should handle with
    // FB in order to avoid tearing
    if ((usage & GRALLOC_USAGE_SW_WRITE_OFTEN) &&
         (usage & GRALLOC_USAGE_SW_READ_OFTEN))
        return false;

    if (!(usage & GRALLOC_USAGE_HW_COMPOSER))
        return false;

    return true;
}

// Check the data format of a buffer
// only following formats were accepted:
// TODO: list valid formats
bool IntelHWComposer::isHWCFormat(int format)
{
    return true;
}

// Check the transform of a layer
// we can only support 180 degree rotation
// TODO: list all supported transforms
// FIXME: for video playback we need to ignore this,
// since video decoding engine will take care of rotation.
bool IntelHWComposer::isHWCTransform(uint32_t transform)
{
    return false;
}

// Check the blending of a layer
// Currently, no blending support
bool IntelHWComposer::isHWCBlending(uint32_t blending)
{
    return false;
}

// Check whether a layer can be handle by our HWC
// if HWC_SKIP_LAYER was set, skip this layer
// TODO: add more
bool IntelHWComposer::isHWCLayer(hwc_layer_t *layer)
{
    if (!layer)
        return false;

    // if (layer->flags & HWC_SKIP_LAYER)
    //    return false;

    // check transform
    // if (!isHWCTransform(layer->transform))
    //    return false;

    // check blending
    // if (!isHWCBlending(layer->blending))
    //    return false;

    intel_gralloc_buffer_handle_t *grallocHandle =
        (intel_gralloc_buffer_handle_t*)layer->handle;

    if (!grallocHandle)
        return false;

    // check buffer usage
    if (!isHWCUsage(grallocHandle->usage))
        return false;

    // check format
    // if (!isHWCFormat(grallocHandle->format))
    //    return false;

    return true;
}

// Check whehter two layers are intersect
bool IntelHWComposer::areLayersIntersecting(hwc_layer_t *top,
                                            hwc_layer_t* bottom)
{
    if (!top || !bottom)
        return false;

    hwc_rect_t *topRect = &top->displayFrame;
    hwc_rect_t *bottomRect = &bottom->displayFrame;

    if (bottomRect->right <= topRect->left ||
        bottomRect->left >= topRect->right ||
        bottomRect->top >= topRect->bottom ||
        bottomRect->bottom <= topRect->top)
        return false;

    return true;
}

void IntelHWComposer::handleHotplugEvent()
{
    LOGD_IF(ALLOW_HWC_PRINT, "handleHotplugEvent");

    // go through layer list and call plane's onModeChange()
    for (size_t i = 0 ; i < mLayerList->getLayersCount(); i++) {
        IntelDisplayPlane *plane = mLayerList->getPlane(i);
        if (plane)
            plane->onDrmModeChange();
    }
}

void IntelHWComposer::onUEvent(const char *msg, int msgLen, int msgType)
{
    android::Mutex::Autolock _l(mLock);

#ifdef TARGET_HAS_MULTIPLE_DISPLAY
    // if mds sent orientation change message, inform widi plane and return
    if (msgType == IntelExternalDisplayMonitor::MSG_TYPE_MDS_ORIENTATION_CHANGE) {
        LOGD("%s: got multiDisplay service orientation change event\n", __func__);
        if(mPlaneManager->isWidiActive()) {
            IntelWidiPlane* widiPlane = (IntelWidiPlane*)mPlaneManager->getWidiPlane();
            if (widiPlane->setOrientationChanged() != NO_ERROR) {
                LOGE("%s: error in sending orientation change event to widiplane");
            }
        }
    }
    // if send by mds, set the hotplug event and return
    if (msgType == IntelExternalDisplayMonitor::MSG_TYPE_MDS) {
        LOGD("%s: got multiDisplay service event\n", __func__);
        mDrm->detectDrmModeInfo();
        handleHotplugEvent();
        mHotplugEvent = true;
        return;
    }
#endif

    if (strcmp(msg, "change@/devices/pci0000:00/0000:00:02.0/drm/card0"))
        return;
    msg += strlen(msg) + 1;

    do {
        if (!strncmp(msg, "HOTPLUG=1", strlen("HOTPLUG=1"))) {
            LOGD("%s: detected hdmi hotplug event\n", __func__);
            mDrm->detectDrmModeInfo();
            handleHotplugEvent();
            mHotplugEvent = true;
            break;
        }
        msg += strlen(msg) + 1;
    } while (*msg);
}

void IntelHWComposer::vsync(int64_t timestamp, int pipe)
{
    if (mProcs && mProcs->vsync) {
        LOGV("%s: report vsync timestamp %llu, pipe %d, active 0x%x", __func__,
             timestamp, pipe, mActiveVsyncs);
        if ((1 << pipe) & mActiveVsyncs)
	    mProcs->vsync(const_cast<hwc_procs_t*>(mProcs), 0, timestamp);
    }
    mLastVsync = timestamp;
}

bool IntelHWComposer::flipFramebufferContexts(void *contexts)
{
    LOGD_IF(ALLOW_HWC_PRINT, "flipFrameBufferContexts");
    intel_sprite_context_t *context;
    mdfld_plane_contexts_t *planeContexts;
    uint32_t fbWidth, fbHeight;
    int zOrderConfig;
    bool forceBottom = false;

    if (!contexts) {
        LOGE("%s: Invalid plane contexts\n", __func__);
        return false;
    }

    planeContexts = (mdfld_plane_contexts_t*)contexts;

    for (int output = 0; output < OUTPUT_MAX; output++) {
        drmModeConnection connection = mDrm->getOutputConnection(output);
        if (connection != DRM_MODE_CONNECTED)
            continue;
        drmModeFBPtr fbInfo = mDrm->getOutputFBInfo(output);

        fbWidth = fbInfo->width;
        fbHeight = fbInfo->height;

        zOrderConfig = mPlaneManager->getZOrderConfig(output);
        if ((zOrderConfig == IntelDisplayPlaneManager::ZORDER_OcOaP) ||
            (zOrderConfig == IntelDisplayPlaneManager::ZORDER_OaOcP))
            forceBottom = true;

        context = &planeContexts->primary_contexts[output];

        // update context
        context->update_mask = SPRITE_UPDATE_ALL;
        context->index = output;
        context->pipe = output;
        context->linoff = 0;
        context->stride = align_to((4 * fbWidth), 64);
        context->pos = 0;
        context->size = ((fbHeight - 1) & 0xfff) << 16 | ((fbWidth - 1) & 0xfff);
        context->surf = 0;

        // config z order; switch z order may cause flicker
        if (forceBottom) {
            context->cntr = INTEL_SPRITE_PIXEL_FORMAT_BGRX8888;
        } else {
            context->cntr = INTEL_SPRITE_PIXEL_FORMAT_BGRA8888;
        }

        context->cntr |= 0x80000000;

        // update active primary
        planeContexts->active_primaries |= (1 << output);
    }

    return true;
}

bool IntelHWComposer::prepare(hwc_layer_list_t *list)
{
    LOGD_IF(ALLOW_HWC_PRINT, "%s\n", __func__);

    if (!initCheck()) {
        LOGE("%s: failed to initialize HWComposer\n", __func__);
        return false;
    }

    android::Mutex::Autolock _l(mLock);

    // delay 3 circles to disable useless overlay planes
    static int counter = 0;
    if (mPlaneManager->hasReclaimedOverlays()) {
        if (++counter == 3) {
            mPlaneManager->disableReclaimedPlanes(IntelDisplayPlane::DISPLAY_PLANE_OVERLAY);
            counter = 0;
        }
    }
    else
        counter = 0;


    // clear force swap buffer flag
    mForceSwapBuffer = false;

    bool widiStatusChanged = mPlaneManager->isWidiStatusChanged();

    // handle geometry changing. attach display planes to layers
    // which can be handled by HWC.
    // plane control information (e.g. position) will be set here
    if (!list || (list->flags & HWC_GEOMETRY_CHANGED) || mHotplugEvent
        || widiStatusChanged) {
        onGeometryChanged(list);

        IntelWidiPlane* widiPlane = (IntelWidiPlane*)mPlaneManager->getWidiPlane();
        if (widiStatusChanged && mDrm) {
            if(mPlaneManager->isWidiActive()) {
                mDrm->notifyWidi(true);
                mDrm->notifyMipi(!widiPlane->isStreaming());
            }
            else
            {
                mDrm->notifyWidi(false);
            }
        }

        if(mHotplugEvent) {
            if(mPlaneManager->isWidiActive()) {
                if(widiPlane->isExtVideoAllowed()) {
                    // default fps to 0. widi stack will decide what correct fps should be
                    int displayW = 0, displayH = 0, fps = 0, isInterlace = 0;
                    if(mDrm->isVideoPlaying()) {
                        if(mDrm->getVideoInfo(&displayW, &displayH, &fps, &isInterlace)) {
                            if(fps < 0) fps = 0;
                        }
                    }
                    widiPlane->setPlayerStatus(mDrm->isVideoPlaying(), fps);
                }
            }
            mHotplugEvent = false;
        }

        intel_overlay_mode_t mode = mDrm->getDisplayMode();
        if (list && (mode == OVERLAY_EXTEND ||  widiPlane->isStreaming()) &&
            (list->flags & HWC_GEOMETRY_CHANGED)) {
            bool hasSkipLayer = false;
            LOGD_IF(ALLOW_HWC_PRINT,
                    "layers num:%d", list->numHwLayers);
            for (size_t j = 0 ; j < list->numHwLayers ; j++) {
                if (list->hwLayers[j].flags & HWC_SKIP_LAYER) {
                hasSkipLayer = true;
                break;
                }
            }

            if (!hasSkipLayer) {
                if (list->numHwLayers == 1)
                    mDrm->notifyMipi(false);
                else
                    mDrm->notifyMipi(true);
            }
        }
    }

    // handle buffer changing. setup data buffer.
    if (list && !updateLayersData(list)) {
        LOGD_IF(ALLOW_HWC_PRINT, "prepare: revisiting layer list\n");
        revisitLayerList(list, false);
    }

    //FIXME: we have to clear fb by hwc itself because
    //surfaceflinger can't do proper clear now.
    if (mForceSwapBuffer)
        mLayerList->clearWithOpenGL();

    return true;
}

bool IntelHWComposer::commit(hwc_display_t dpy,
                             hwc_surface_t sur,
                             hwc_layer_list_t *list)
{
    LOGD_IF(ALLOW_HWC_PRINT, "%s\n", __func__);

    if (!initCheck()) {
        LOGE("%s: failed to initialize HWComposer\n", __func__);
        return false;
    }

    android::Mutex::Autolock _l(mLock);

    // if hotplug was happened & didn't be handled skip the flip
    if (mHotplugEvent)
        return true;

    void *context = mPlaneManager->getPlaneContexts();
    if (!context) {
        LOGE("%s: invalid plane contexts\n", __func__);
        return false;
    }

    // need check whether eglSwapBuffers is necessary
    bool needSwapBuffer = false;

    // if all layers were attached with display planes then we don't need
    // swap buffers.
    if (!mLayerList->getLayersCount() ||
        mLayerList->getLayersCount() != mLayerList->getAttachedPlanesCount() ||
        mForceSwapBuffer) {
        LOGD_IF(ALLOW_HWC_PRINT,
               "%s: mForceSwapBuffer: %d, layer count: %d, attached plane:%d\n",
               __func__, mForceSwapBuffer, mLayerList->getLayersCount(),
               mLayerList->getAttachedPlanesCount());

        // FIXME: it might be a surface flinger bug
        // surface flinger failed to render a layer to FB sometimes
	// because screen dirty region was unchanged, in this case
        // we don't to swap buffers.
#ifdef INTEL_EXT_SF_NEED_SWAPBUFFER
        if (!list || (list->flags & HWC_NEED_SWAPBUFFERS))
#endif
            needSwapBuffer = true;
    }

    // if primary plane is in use, skip eglSwapBuffers
    if (!mPlaneManager->primaryAvailable(0)) {
        LOGD_IF(ALLOW_HWC_PRINT, "%s: primary plane in use\n", __func__);
        needSwapBuffer = false;
    }

    // check whether eglSwapBuffers is still needed for the given layer list
    if (needSwapBuffer) {
        LOGD_IF(ALLOW_HWC_PRINT, "%s: eglSwapBuffers\n", __func__);
        EGLBoolean sucess = eglSwapBuffers((EGLDisplay)dpy, (EGLSurface)sur);
        if (!sucess) {
            return false;
        }
    }

    if(mPlaneManager->isWidiActive()) {
        IntelDisplayPlane *p = mPlaneManager->getWidiPlane();
        LOGD_IF(ALLOW_HWC_PRINT, "Widi Plane is %p",p);
         if (p)
             p->flip(context, 0);
         else
             LOGE("Widi Plane is NULL");
    }

    if (mFBDev->bBypassPost) {
        buffer_handle_t bufferHandles[INTEL_DISPLAY_PLANE_NUM];
        int numBuffers = 0;
        // setup primary plane contexts if swap buffers is needed
        if (needSwapBuffer) {
            flipFramebufferContexts(context);
            bufferHandles[numBuffers++] = 0;
        }

        // Call plane's flip for each layer in hwc_layer_list, if a plane has
        // been attached to a layer
        // First post RGB layers, then overlay layers.
        for (size_t i=0 ; list && i<list->numHwLayers ; i++) {
            IntelDisplayPlane *plane = mLayerList->getPlane(i);
            int flags = mLayerList->getFlags(i);

            if (!plane)
                continue;
            if (list->hwLayers[i].flags & HWC_SKIP_LAYER)
                continue;
            if (list->hwLayers[i].compositionType != HWC_OVERLAY)
                continue;

            LOGD_IF(ALLOW_HWC_PRINT, "%s: flip plane %d, flags: 0x%x\n",
                __func__, i, flags);

            bool ret = plane->flip(context, flags);
            if (!ret)
                LOGW("%s: failed to flip plane %d context !\n", __func__, i);
            else
                bufferHandles[numBuffers++] =
                (buffer_handle_t)plane->getDataBufferHandle();

            // clear flip flags, except for DELAY_DISABLE
            mLayerList->setFlags(i, flags & IntelDisplayPlane::DELAY_DISABLE);

            // remove clear fb hints
            list->hwLayers[i].hints &= ~HWC_HINT_CLEAR_FB;
        }

        // commit plane contexts
        if (mFBDev && numBuffers) {
            LOGD_IF(ALLOW_HWC_PRINT, "%s: commits %d buffers\n", __func__, numBuffers);
            int err = mFBDev->Post2(&mFBDev->base,
                                    bufferHandles,
                                    numBuffers,
                                    context,
                                    mPlaneManager->getContextLength());
            if (err) {
                LOGE("%s: Post2 failed with errno %d\n", __func__, err);
                return false;
            }
        }
    }

        //make sure all flips were finished
        for (size_t i=0 ; list && i<list->numHwLayers ; i++) {
            IntelDisplayPlane *plane = mLayerList->getPlane(i);
            int flags = mLayerList->getFlags(i);
            if (!plane)
                continue;
            if (list->hwLayers[i].flags & HWC_SKIP_LAYER)
                continue;
            if (list->hwLayers[i].compositionType != HWC_OVERLAY)
                continue;

            plane->waitForFlipCompletion();
        }

    return true;
}

uint32_t IntelHWComposer::disableUnusedVsyncs(uint32_t target)
{
    uint32_t unusedVsyncs = mActiveVsyncs & (~target);
    struct drm_psb_vsync_set_arg arg;
    uint32_t vsync;
    int i, ret;

    LOGV("disableVsync: unusedVsyncs 0x%x\n", unusedVsyncs);

    if (!unusedVsyncs)
        goto disable_out;

    /*disable unused vsyncs*/
    for (i = 0; i < VSYNC_SRC_NUM; i++) {
        vsync = (1 << i);
        if (!(vsync & unusedVsyncs))
            continue;

        /*disable vsync*/
        if (i == VSYNC_SRC_FAKE)
            mFakeVsync->setEnabled(false, mLastVsync);
        else {
            memset(&arg, 0, sizeof(struct drm_psb_vsync_set_arg));
            arg.vsync_operation_mask = VSYNC_DISABLE;

            // pipe select
            if (i == VSYNC_SRC_HDMI)
                arg.vsync.pipe = 1;
            else
                arg.vsync.pipe = 0;

            ret = drmCommandWriteRead(mDrm->getDrmFd(), DRM_PSB_VSYNC_SET,
                                      &arg, sizeof(arg));
            if (ret) {
                LOGW("%s: failed to enable/disable vsync %d\n", __func__, ret);
                continue;
            }
            mVsyncsEnabled = 0;
        }

        /*disabled successfully, remove it from unused vsyncs*/
        unusedVsyncs &= ~vsync;
    }
disable_out:
    return unusedVsyncs;
}

uint32_t IntelHWComposer::enableVsyncs(uint32_t target)
{
    uint32_t enabledVsyncs = 0;
    struct drm_psb_vsync_set_arg arg;
    uint32_t vsync;
    int i, ret;

    LOGV("enableVsyn: enable vsyncs 0x%x\n", target);

    if (!target) {
        enabledVsyncs = 0;
        goto enable_out;
    }

    // remove all active vsyncs from target
    target &= ~mActiveVsyncs;
    if (!target) {
        enabledVsyncs = mActiveVsyncs;
        goto enable_out;
    }

    // enable vsyncs which is currently inactive
    for (i = 0; i < VSYNC_SRC_NUM; i++) {
        vsync = (1 << i);
        if (!(vsync & target))
            continue;

        /*enable vsync*/
        if (i == VSYNC_SRC_FAKE)
            mFakeVsync->setEnabled(true, mLastVsync);
        else {
            memset(&arg, 0, sizeof(struct drm_psb_vsync_set_arg));
            arg.vsync_operation_mask = VSYNC_ENABLE;

            // pipe select
            if (i == VSYNC_SRC_HDMI)
                arg.vsync.pipe = 1;
            else
                arg.vsync.pipe = 0;

            ret = drmCommandWriteRead(mDrm->getDrmFd(), DRM_PSB_VSYNC_SET,
                                      &arg, sizeof(arg));
            if (ret) {
                LOGW("%s: failed to enable vsync %d\n", __func__, ret);
                continue;
            }
            mVsyncsEnabled = 1;
        }

        /*enabled successfully*/
        enabledVsyncs |= vsync;
    }
enable_out:
    return enabledVsyncs;
}

bool IntelHWComposer::vsyncControl(int enabled)
{
    uint32_t targetVsyncs = 0;
    uint32_t activeVsyncs = 0;
    uint32_t enabledVsyncs = 0;
    IntelWidiPlane* widiPlane = 0;

    LOGV("vsyncControl, enabled %d\n", enabled);

    if (enabled != 0 && enabled != 1)
        return false;

    android::Mutex::Autolock _l(mLock);

    // for disable vsync request, disable all active vsyncs
    if (!enabled) {
        targetVsyncs = 0;
        goto disable_vsyncs;
    }

    // use fake vsync for widi extend video mode
    widiPlane = (IntelWidiPlane*)mPlaneManager->getWidiPlane();
    if (widiPlane && widiPlane->isActive() &&
        widiPlane->isExtVideoAllowed() &&
        widiPlane->isPlayerOn()) {
        targetVsyncs |= (1 << VSYNC_SRC_FAKE);
    } else if (OVERLAY_EXTEND == mDrm->getDisplayMode()) {
        targetVsyncs |= (1 << VSYNC_SRC_HDMI);
    } else
        targetVsyncs |= (1 << VSYNC_SRC_MIPI);

    // enable selected vsyncs
    enabledVsyncs = enableVsyncs(targetVsyncs);

disable_vsyncs:
    // disable unused vsyncs
    activeVsyncs = disableUnusedVsyncs(targetVsyncs);

    // update active vsyncs
    mActiveVsyncs = enabledVsyncs | activeVsyncs;
    mVsync->setActiveVsyncs(mActiveVsyncs);

    LOGV("vsyncControl: activeVsyncs 0x%x\n", mActiveVsyncs);
    return true;
}

bool IntelHWComposer::release()
{
    LOGD("release");

    if (!initCheck() || !mLayerList)
        return false;

    // disable all attached planes
    for (size_t i=0 ; i<mLayerList->getLayersCount() ; i++) {
        IntelDisplayPlane *plane = mLayerList->getPlane(i);

        if (plane) {
            // disable all attached planes
            plane->disable();

            // release all data buffers
            plane->invalidateDataBuffer();
        }
    }

    return true;
}

bool IntelHWComposer::dump(char *buff,
                           int buff_len, int *cur_len)
{
    IntelDisplayPlane *plane = NULL;
    bool ret = true;
    int i;

    mDumpBuf = buff;
    mDumpBuflen = buff_len;
    mDumpLen = 0;

    IntelWidiPlane* widiPlane = (IntelWidiPlane*)mPlaneManager->getWidiPlane();

    if (mLayerList) {
       dumpPrintf("------------ Totally %d layers -------------\n",
                                     mLayerList->getLayersCount());
       for (i = 0; i < mLayerList->getLayersCount(); i++) {
           plane = mLayerList->getPlane(i);

           if (plane) {
               int planeType = plane->getPlaneType();
               dumpPrintf("   # layer %d attached to %s plane \n", i,
                            (planeType == 3) ? "overlay" : "sprite");
           } else
               dumpPrintf("   # layer %d goes through eglswapbuffer\n ", i);
       }

       dumpPrintf("-------------runtime parameters -------------\n");
       dumpPrintf("  + bypassPost: %d \n", mFBDev->bBypassPost);
       dumpPrintf("  + mForceSwapBuffer: %d \n", mForceSwapBuffer);
       dumpPrintf("  + Display Mode: %d \n", mDrm->getDisplayMode());
       dumpPrintf("  + isHdmiConnected: %d \n",
        (mDrm->getOutputConnection(OUTPUT_HDMI) == DRM_MODE_CONNECTED) ? 1 : 0);
       dumpPrintf("  + isWidiActive: %d \n", (widiPlane->isActive()) ? 1 : 0);
       dumpPrintf("  + mActiveVsyncs: 0x%x, mVsyncsEnabled: %d \n", mActiveVsyncs, mVsyncsEnabled);
    }

    mPlaneManager->dump(mDumpBuf,  mDumpBuflen, &mDumpLen);

    return ret;
}

bool IntelHWComposer::initialize()
{
    bool ret = true;

    //TODO: replace the hard code buffer type later
    int bufferType = IntelBufferManager::TTM_BUFFER;

    LOGD_IF(ALLOW_HWC_PRINT, "%s\n", __func__);

    // open IMG frame buffer device
    hw_module_t const* module;
    if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module) == 0) {
        IMG_gralloc_module_public_t *imgGrallocModule;
        imgGrallocModule = (IMG_gralloc_module_public_t*)module;
        mFBDev = imgGrallocModule->psFrameBufferDevice;
        mFBDev->bBypassPost = cfg.bypasspost;
    }

    if (!mFBDev) {
        LOGE("%s: failed to open IMG FB device\n", __func__);
        return false;
    }

    //create new DRM object if not exists
    if (!mDrm) {
        mDrm = &IntelHWComposerDrm::getInstance();
        if (!mDrm) {
            LOGE("%s: Invalid DRM object\n", __func__);
            ret = false;
            goto drm_err;
        }

        ret = mDrm->initialize(this);
        if (ret == false) {
            LOGE("%s: failed to initialize DRM instance\n", __func__);
            goto drm_err;
        }
    }

    mVsync = new IntelVsyncEventHandler(this, mDrm->getDrmFd());

    mFakeVsync = new IntelFakeVsyncEvent(this);

    //create new buffer manager and initialize it
    if (!mBufferManager) {
        //mBufferManager = new IntelTTMBufferManager(mDrm->getDrmFd());
	mBufferManager = new IntelBCDBufferManager(mDrm->getDrmFd());
        if (!mBufferManager) {
            LOGE("%s: Failed to create buffer manager\n", __func__);
            ret = false;
            goto bm_err;
        }
        // do initialization
        ret = mBufferManager->initialize();
        if (ret == false) {
            LOGE("%s: Failed to initialize buffer manager\n", __func__);
            goto bm_init_err;
        }
    }

    // create buffer manager for gralloc buffer
    if (!mGrallocBufferManager) {
        //mGrallocBufferManager = new IntelPVRBufferManager(mDrm->getDrmFd());
	mGrallocBufferManager = new IntelGraphicBufferManager(mDrm->getDrmFd());
        if (!mGrallocBufferManager) {
            LOGE("%s: Failed to create Gralloc buffer manager\n", __func__);
            ret = false;
            goto gralloc_bm_err;
        }

        ret = mGrallocBufferManager->initialize();
        if (ret == false) {
            LOGE("%s: Failed to initialize Gralloc buffer manager\n", __func__);
            goto gralloc_bm_err;
        }
    }

    // create new display plane manager
    if (!mPlaneManager) {
        mPlaneManager =
            new IntelDisplayPlaneManager(mDrm->getDrmFd(),
                                         mBufferManager, mGrallocBufferManager);
        if (!mPlaneManager) {
            LOGE("%s: Failed to create plane manager\n", __func__);
            goto bm_init_err;
        }
    }

    // create layer list
    mLayerList = new IntelHWComposerLayerList(mPlaneManager);
    if (!mLayerList) {
        LOGE("%s: Failed to create layer list\n", __func__);
        goto pm_err;
    }

    mInitialized = true;

    LOGD_IF(ALLOW_HWC_PRINT, "%s: successfully\n", __func__);
    return true;

pm_err:
    delete mPlaneManager;
bm_init_err:
    delete mGrallocBufferManager;
gralloc_bm_err:
    delete mBufferManager;
    mBufferManager = 0;
bm_err:
    stopObserver();
observer_err:
    delete mDrm;
    mDrm = 0;
drm_err:
    return ret;
}


#include <dix-config.h>

#include "dix/dix_priv.h"
#include "dix/screenint_priv.h"

#include <dix.h>
#include "vndserver.h"

// HACK: The opcode in old glxproto.h has a typo in it.
#if !defined(X_GLXCreateContextAttribsARB)
#define X_GLXCreateContextAttribsARB X_GLXCreateContextAtrribsARB
#endif

static GlxServerVendor *vendorForScreen(
    ClientPtr pClient,
    CARD32 raw_screen)
{
    CARD32 screenId = GlxCheckSwap(pClient, raw_screen);
    ScreenPtr pScreen = GetScreenPtr(screenId);
    if (!pScreen) {
        pClient->errorValue = screenId;
        return NULL;
    }

    return glxServer.getVendorForScreen(pClient, pScreen);
}

static GlxServerVendor *vendorForTag(
    ClientPtr pClient,
    CARD32 raw_context)
{
    CARD32 contextTag = GlxCheckSwap(client, raw_context);
    GlxServerVendor *vendor = glxServer.getContextTag(client, contextTag);
    if (!vendor) {
        client->errorValue = contextTag;
        return NULL;
    }

    return vendor;
}

static int forward_simple(ClientPtr pClient, CARD32 raw_screen)
{
    GlxServerVendor *vendor = vendorForScreen(pClient, raw_screen);
    if (!vendor)
        return BadMatch;

    return glxServer.forwardRequest(vendor, pClient);
}

static int forward_xidmap(ClientPtr pClient, CARD32 raw_screen, XID xid)
{
    GlxServerVendor *vendor = vendorForScreen(pClient, raw_screen);
    if (!vendor)
        return BadMatch;

    if (!glxServer.addXIDMap(xid, vendor)) {
        return BadAlloc;
    }

    int ret = glxServer.forwardRequest(vendor, pClient);
    if (ret != Success) {
        glxServer.removeXIDMap(xid);
    }

    return ret;
}

static int forward_create(ClientPtr pClient, CARD32 raw_screen, XID raw_xid)
{
    XID xid = GlxCheckSwap(pClient, raw_xid);
    LEGAL_NEW_RESOURCE(xid, pClient);

    return forward_xidmap(pClient, raw_screen, xid);
}

static int dispatch_Render(ClientPtr client)
{
    REQUEST(xGLXRenderReq);
    CARD32 contextTag;
    GlxServerVendor *vendor = NULL;
    REQUEST_AT_LEAST_SIZE(*stuff);
    contextTag = GlxCheckSwap(client, stuff->contextTag);

    GlxServerVendor *vendor = glxServer.getContextTag(client, stuff->contextTag);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = contextTag;
        return GlxErrorBase + GLXBadContextTag;
    }
}

static int dispatch_RenderLarge(ClientPtr client)
{
    REQUEST(xGLXRenderLargeReq);
    CARD32 contextTag;
    GlxServerVendor *vendor = NULL;
    REQUEST_AT_LEAST_SIZE(*stuff);
    contextTag = GlxCheckSwap(client, stuff->contextTag);
    vendor = glxServer.getContextTag(client, contextTag);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = contextTag;
        return GlxErrorBase + GLXBadContextTag;
    }
}

static int dispatch_CreateContext(ClientPtr client)
{
    REQUEST(xGLXCreateContextReq);
    REQUEST_SIZE_MATCH(*stuff);

    return forward_create(client, stuff->screen, stuff->context);
}

static int dispatch_DestroyContext(ClientPtr client)
{
    REQUEST(xGLXDestroyContextReq);
    CARD32 context;
    GlxServerVendor *vendor = NULL;
    REQUEST_SIZE_MATCH(*stuff);
    context = GlxCheckSwap(client, stuff->context);
    vendor = glxServer.getXIDMap(context);

    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        if (ret == Success) {
            glxServer.removeXIDMap(context);
        }
        return ret;
    } else {
        client->errorValue = context;
        return GlxErrorBase + GLXBadContext;
    }
}
static int dispatch_WaitGL(ClientPtr client)
{
    REQUEST(xGLXWaitGLReq);
    CARD32 contextTag;
    GlxServerVendor *vendor = NULL;
    REQUEST_SIZE_MATCH(*stuff);
    contextTag = GlxCheckSwap(client, stuff->contextTag);
    vendor = glxServer.getContextTag(client, contextTag);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = contextTag;
        return GlxErrorBase + GLXBadContextTag;
    }
}
static int dispatch_WaitX(ClientPtr client)
{
    REQUEST(xGLXWaitXReq);
    CARD32 contextTag;
    GlxServerVendor *vendor = NULL;
    REQUEST_SIZE_MATCH(*stuff);
    contextTag = GlxCheckSwap(client, stuff->contextTag);
    vendor = glxServer.getContextTag(client, contextTag);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = contextTag;
        return GlxErrorBase + GLXBadContextTag;
    }
}
static int dispatch_UseXFont(ClientPtr client)
{
    REQUEST(xGLXUseXFontReq);
    CARD32 contextTag;
    GlxServerVendor *vendor = NULL;
    REQUEST_SIZE_MATCH(*stuff);
    contextTag = GlxCheckSwap(client, stuff->contextTag);
    vendor = glxServer.getContextTag(client, contextTag);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = contextTag;
        return GlxErrorBase + GLXBadContextTag;
    }
}
static int dispatch_CreateGLXPixmap(ClientPtr client)
{
    REQUEST(xGLXCreateGLXPixmapReq);
    REQUEST_SIZE_MATCH(*stuff);

    return forward_xidmap(client, stuff->screen, stuff->glxpixmap);
}

static int dispatch_GetVisualConfigs(ClientPtr client)
{
    REQUEST(xGLXGetVisualConfigsReq);
    REQUEST_SIZE_MATCH(*stuff);

    return forward_simple(client, stuff->screen);
}

static int dispatch_DestroyGLXPixmap(ClientPtr client)
{
    REQUEST(xGLXDestroyGLXPixmapReq);
    CARD32 glxpixmap;
    GlxServerVendor *vendor = NULL;
    REQUEST_SIZE_MATCH(*stuff);
    glxpixmap = GlxCheckSwap(client, stuff->glxpixmap);
    vendor = glxServer.getXIDMap(glxpixmap);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = glxpixmap;
        return GlxErrorBase + GLXBadPixmap;
    }
}

static int dispatch_QueryExtensionsString(ClientPtr client)
{
    REQUEST(xGLXQueryExtensionsStringReq);
    REQUEST_SIZE_MATCH(*stuff);

    return forward_simple(client, stuff->screen);
}

static int dispatch_QueryServerString(ClientPtr client)
{
    REQUEST(xGLXQueryServerStringReq);
    REQUEST_SIZE_MATCH(*stuff);

    return forward_simple(client, stuff->screen);
}

static int dispatch_ChangeDrawableAttributes(ClientPtr client)
{
    REQUEST(xGLXChangeDrawableAttributesReq);
    CARD32 drawable;
    GlxServerVendor *vendor = NULL;
    REQUEST_AT_LEAST_SIZE(*stuff);
    drawable = GlxCheckSwap(client, stuff->drawable);
    vendor = glxServer.getXIDMap(drawable);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = drawable;
        return BadDrawable;
    }
}
static int dispatch_CreateNewContext(ClientPtr client)
{
    REQUEST(xGLXCreateNewContextReq);
    REQUEST_SIZE_MATCH(*stuff);

    return forward_xidmap(client, stuff->screen, stuff->context);
}

static int dispatch_CreatePbuffer(ClientPtr client)
{
    REQUEST(xGLXCreatePbufferReq);
    REQUEST_AT_LEAST_SIZE(*stuff);

    return forward_xidmap(client, stuff->screen, stuff->pbuffer);
}

static int dispatch_CreatePixmap(ClientPtr client)
{
    REQUEST(xGLXCreatePixmapReq);
    REQUEST_AT_LEAST_SIZE(*stuff);

    return forward_xidmap(client, stuff->screen, stuff->glxpixmap);
}

static int dispatch_CreateWindow(ClientPtr client)
{
    REQUEST(xGLXCreateWindowReq);
    REQUEST_AT_LEAST_SIZE(*stuff);

    return forward_xidmap(client, stuff->screen, stuff->glxwindow);
}

static int dispatch_CreateContextAttribsARB(ClientPtr client)
{
    REQUEST(xGLXCreateContextAttribsARBReq);
    REQUEST_AT_LEAST_SIZE(*stuff);

    return forward_xidmap(client, stuff->screen, stuff->context);
}

static int dispatch_DestroyPbuffer(ClientPtr client)
{
    REQUEST(xGLXDestroyPbufferReq);
    CARD32 pbuffer;
    GlxServerVendor *vendor = NULL;
    REQUEST_SIZE_MATCH(*stuff);
    pbuffer = GlxCheckSwap(client, stuff->pbuffer);
    vendor = glxServer.getXIDMap(pbuffer);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        if (ret == Success) {
            glxServer.removeXIDMap(pbuffer);
        }
        return ret;
    } else {
        client->errorValue = pbuffer;
        return GlxErrorBase + GLXBadPbuffer;
    }
}
static int dispatch_DestroyPixmap(ClientPtr client)
{
    REQUEST(xGLXDestroyPixmapReq);
    CARD32 glxpixmap;
    GlxServerVendor *vendor = NULL;
    REQUEST_SIZE_MATCH(*stuff);
    glxpixmap = GlxCheckSwap(client, stuff->glxpixmap);
    vendor = glxServer.getXIDMap(glxpixmap);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        if (ret == Success) {
            glxServer.removeXIDMap(glxpixmap);
        }
        return ret;
    } else {
        client->errorValue = glxpixmap;
        return GlxErrorBase + GLXBadPixmap;
    }
}
static int dispatch_DestroyWindow(ClientPtr client)
{
    REQUEST(xGLXDestroyWindowReq);
    CARD32 glxwindow;
    GlxServerVendor *vendor = NULL;
    REQUEST_SIZE_MATCH(*stuff);
    glxwindow = GlxCheckSwap(client, stuff->glxwindow);
    vendor = glxServer.getXIDMap(glxwindow);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        if (ret == Success) {
            glxServer.removeXIDMap(glxwindow);
        }
        return ret;
    } else {
        client->errorValue = glxwindow;
        return GlxErrorBase + GLXBadWindow;
    }
}
static int dispatch_GetDrawableAttributes(ClientPtr client)
{
    REQUEST(xGLXGetDrawableAttributesReq);
    CARD32 drawable;
    GlxServerVendor *vendor = NULL;
    REQUEST_SIZE_MATCH(*stuff);
    drawable = GlxCheckSwap(client, stuff->drawable);
    vendor = glxServer.getXIDMap(drawable);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = drawable;
        return BadDrawable;
    }
}
static int dispatch_GetFBConfigs(ClientPtr client)
{
    REQUEST(xGLXGetFBConfigsReq);
    REQUEST_SIZE_MATCH(*stuff);

    return forward_simple(client, stuff->screen);
}

static int dispatch_QueryContext(ClientPtr client)
{
    REQUEST(xGLXQueryContextReq);
    CARD32 context;
    GlxServerVendor *vendor = NULL;
    REQUEST_SIZE_MATCH(*stuff);
    context = GlxCheckSwap(client, stuff->context);
    vendor = glxServer.getXIDMap(context);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = context;
        return GlxErrorBase + GLXBadContext;
    }
}
static int dispatch_IsDirect(ClientPtr client)
{
    REQUEST(xGLXIsDirectReq);
    CARD32 context;
    GlxServerVendor *vendor = NULL;
    REQUEST_SIZE_MATCH(*stuff);
    context = GlxCheckSwap(client, stuff->context);
    vendor = glxServer.getXIDMap(context);
    if (vendor != NULL) {
        int ret;
        ret = glxServer.forwardRequest(vendor, client);
        return ret;
    } else {
        client->errorValue = context;
        return GlxErrorBase + GLXBadContext;
    }
}

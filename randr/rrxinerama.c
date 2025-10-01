/*
 * Copyright © 2006 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */
/*
 * This Xinerama implementation comes from the SiS driver which has
 * the following notice:
 */
/*
 * SiS driver main code
 *
 * Copyright (C) 2001-2005 by Thomas Winischhofer, Vienna, Austria.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1) Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2) Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3) The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Thomas Winischhofer <thomas@winischhofer.net>
 *	- driver entirely rewritten since 2001, only basic structure taken from
 *	  old code (except sis_dri.c, sis_shadow.c, sis_accel.c and parts of
 *	  sis_dga.c; these were mostly taken over; sis_dri.c was changed for
 *	  new versions of the DRI layer)
 *
 * This notice covers the entire driver code unless indicated otherwise.
 *
 * Formerly based on code which was
 * 	     Copyright (C) 1998, 1999 by Alan Hourihane, Wigan, England.
 * 	     Written by:
 *           Alan Hourihane <alanh@fairlite.demon.co.uk>,
 *           Mike Chapman <mike@paranoia.com>,
 *           Juanjo Santamarta <santamarta@ctv.es>,
 *           Mitani Hiroshi <hmitani@drl.mei.co.jp>,
 *           David Thomas <davtom@dream.org.uk>.
 */
#include <dix-config.h>

#include <X11/Xmd.h>
#include <X11/extensions/panoramiXproto.h>

#include "dix/dix_priv.h"
#include "dix/request_priv.h"
#include "dix/screenint_priv.h"
#include "include/extinit.h"
#include "randr/randrstr_priv.h"

#include "swaprep.h"
#include "protocol-versions.h"

/* Xinerama is not multi-screen capable; just report about screen 0 */
#define RR_XINERAMA_SCREEN  0

static int ProcRRXineramaQueryVersion(ClientPtr client);
static int ProcRRXineramaGetState(ClientPtr client);
static int ProcRRXineramaGetScreenCount(ClientPtr client);
static int ProcRRXineramaGetScreenSize(ClientPtr client);
static int ProcRRXineramaIsActive(ClientPtr client);
static int ProcRRXineramaQueryScreens(ClientPtr client);

Bool noRRXineramaExtension = FALSE;

/* Proc */

int
ProcRRXineramaQueryVersion(ClientPtr client)
{
    xPanoramiXQueryVersionReply rep = {
        .majorVersion = SERVER_RRXINERAMA_MAJOR_VERSION,
        .minorVersion = SERVER_RRXINERAMA_MINOR_VERSION
    };

    REQUEST_SIZE_MATCH(xPanoramiXQueryVersionReq);
    if (client->swapped) {
        swaps(&rep.majorVersion);
        swaps(&rep.minorVersion);
    }
    return X_SEND_REPLY_SIMPLE(client, rep);
}

int
ProcRRXineramaGetState(ClientPtr client)
{
    REQUEST(xPanoramiXGetStateReq);
    REQUEST_SIZE_MATCH(xPanoramiXGetStateReq);

    if (client->swapped)
        swapl(&stuff->window);

    WindowPtr pWin;
    register int rc;
    ScreenPtr pScreen;
    rrScrPrivPtr pScrPriv;
    Bool active = FALSE;

    rc = dixLookupWindow(&pWin, stuff->window, client, DixGetAttrAccess);
    if (rc != Success)
        return rc;

    pScreen = pWin->drawable.pScreen;
    pScrPriv = rrGetScrPriv(pScreen);
    if (pScrPriv) {
        /* XXX do we need more than this? */
        active = TRUE;
    }

    xPanoramiXGetStateReply rep = {
        .state = active,
        .window = stuff->window
    };
    if (client->swapped) {
        swapl(&rep.window);
    }
    return X_SEND_REPLY_SIMPLE(client, rep);
}

static int
RRXineramaScreenCount(ScreenPtr pScreen)
{
    return RRMonitorCountList(pScreen);
}

static Bool
RRXineramaScreenActive(ScreenPtr pScreen)
{
    return RRXineramaScreenCount(pScreen) > 0;
}

int
ProcRRXineramaGetScreenCount(ClientPtr client)
{
    REQUEST(xPanoramiXGetScreenCountReq);
    REQUEST_SIZE_MATCH(xPanoramiXGetScreenCountReq);

    if (client->swapped)
        swapl(&stuff->window);

    WindowPtr pWin;
    register int rc;

    rc = dixLookupWindow(&pWin, stuff->window, client, DixGetAttrAccess);
    if (rc != Success)
        return rc;

    xPanoramiXGetScreenCountReply rep = {
        .ScreenCount = RRXineramaScreenCount(pWin->drawable.pScreen),
        .window = stuff->window
    };
    if (client->swapped) {
        swapl(&rep.window);
    }
    return X_SEND_REPLY_SIMPLE(client, rep);
}

int
ProcRRXineramaGetScreenSize(ClientPtr client)
{
    REQUEST(xPanoramiXGetScreenSizeReq);
    REQUEST_SIZE_MATCH(xPanoramiXGetScreenSizeReq);

    if (client->swapped) {
        swapl(&stuff->window);
        swapl(&stuff->screen);
    }

    WindowPtr pWin, pRoot;
    ScreenPtr pScreen;
    register int rc;

    rc = dixLookupWindow(&pWin, stuff->window, client, DixGetAttrAccess);
    if (rc != Success)
        return rc;

    pScreen = pWin->drawable.pScreen;
    pRoot = pScreen->root;

    xPanoramiXGetScreenSizeReply rep = {
        .width = pRoot->drawable.width,
        .height = pRoot->drawable.height,
        .window = stuff->window,
        .screen = stuff->screen
    };
    if (client->swapped) {
        swapl(&rep.width);
        swapl(&rep.height);
        swapl(&rep.window);
        swapl(&rep.screen);
    }
    return X_SEND_REPLY_SIMPLE(client, rep);
}

int
ProcRRXineramaIsActive(ClientPtr client)
{
    REQUEST_SIZE_MATCH(xXineramaIsActiveReq);

    xXineramaIsActiveReply rep = {
        .state = RRXineramaScreenActive(screenInfo.screens[RR_XINERAMA_SCREEN])
    };
    if (client->swapped) {
        swapl(&rep.state);
    }
    return X_SEND_REPLY_SIMPLE(client, rep);
}

int
ProcRRXineramaQueryScreens(ClientPtr client)
{
    ScreenPtr pScreen = screenInfo.screens[RR_XINERAMA_SCREEN];
    int m;
    RRMonitorPtr monitors = NULL;
    int nmonitors = 0;

    REQUEST_SIZE_MATCH(xXineramaQueryScreensReq);

    if (RRXineramaScreenActive(pScreen)) {
        RRGetInfo(pScreen, FALSE);
        if (!RRMonitorMakeList(pScreen, TRUE, &monitors, &nmonitors))
            return BadAlloc;
    }

    x_rpcbuf_t rpcbuf = { .swapped = client->swapped, .err_clear = TRUE };

    xXineramaQueryScreensReply rep = {
        .number = nmonitors
    };
    if (client->swapped) {
        swapl(&rep.number);
    }

    for (m = 0; m < nmonitors; m++) {
        BoxRec box = monitors[m].geometry.box;
        /* write xXineramaScreenInfo */
        x_rpcbuf_write_rect(&rpcbuf,
                            box.x1,
                            box.y1,
                            box.x2 - box.x1,
                            box.y2 - box.y1);
    }

    if (monitors)
        RRMonitorFreeList(monitors, nmonitors);

    return X_SEND_REPLY_WITH_RPCBUF(client, rep, rpcbuf);
}

static int
ProcRRXineramaDispatch(ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_PanoramiXQueryVersion:
        return ProcRRXineramaQueryVersion(client);
    case X_PanoramiXGetState:
        return ProcRRXineramaGetState(client);
    case X_PanoramiXGetScreenCount:
        return ProcRRXineramaGetScreenCount(client);
    case X_PanoramiXGetScreenSize:
        return ProcRRXineramaGetScreenSize(client);
    case X_XineramaIsActive:
        return ProcRRXineramaIsActive(client);
    case X_XineramaQueryScreens:
        return ProcRRXineramaQueryScreens(client);
    }
    return BadRequest;
}

void
RRXineramaExtensionInit(void)
{
#ifdef XINERAMA
    if (!noPanoramiXExtension)
        return;
#endif /* XINERAMA */

    if (noRRXineramaExtension)
      return;

    /*
     * Xinerama isn't capable enough to have multiple protocol screens each
     * with their own output geometry.  So if there's more than one protocol
     * screen, just don't even try.
     */
    if (dixGetScreenPtr(1))
        return;

    (void) AddExtension(PANORAMIX_PROTOCOL_NAME, 0, 0,
                        ProcRRXineramaDispatch,
                        ProcRRXineramaDispatch,
                        NULL,
                        StandardMinorOpcode);
}

#pragma once

/**
 * >WM_NOTIFY_DPICHANGED
 * Sent by a top window to its child windows when they are owned by other threads (not by one that has created the top
 * window).
 * The message introduced into the application because `WM_DPICHANGED` can't intersect threads bounds (I don't know
 * why). Neither sending nor posting the message helps.
 * nWParam
 * The HIWORD of the nWParam contains the Y-axis value of the new DPI of the suggested window. The LOWORD of the nWParam
 * contains the X-axis value of the new DPI of the suggested window.
 * nLParam ain't used.
 * Return value is ignored.
 */
#define WM_NOTIFY_DPICHANGED (WM_APP + 0)
/**
 * >WM_ANIMATION_STORYBOARD_FINISHED
 * Informs the target window that a storyboard has finished.
 * nWParam ain't used.
 * nLParam ain't used.
 * Return value is ignored.
 */
#define WM_ANIMATION_STORYBOARD_FINISHED (WM_APP + 1)

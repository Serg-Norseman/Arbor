#pragma once

/**
 * >WM_NOTIFY_DPICHANGED
 * Sent by a top window to its child windows when they are owned by other threads (not by one that has created the top
 * window).
 * The message introduced into the application because `WM_DPICHANGED` can't intersect threads bounds (I don't know
 * why). Neither sending nor posting the message helps.
 * wParam
 * The HIWORD of the wParam contains the Y-axis value of the new DPI of the suggested window. The LOWORD of the wParam
 * contains the X-axis value of the new DPI of the suggested window.
 * lParam ain't used.
 * Return value is ignored.
 */
#define WM_NOTIFY_DPICHANGED (WM_APP + 0)
/**
 * >WM_ANIMATION_STORYBOARD_FINISHED
 * Informs the target window that a storyboard has finished.
 * wParam ain't used.
 * lParam ain't used.
 * Return value is ignored.
 */
#define WM_ANIMATION_STORYBOARD_FINISHED (WM_APP + 1)

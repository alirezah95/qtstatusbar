/*
 * Copyright (c) 2016 J-P Nurmi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include "statusbar_p.h"
#include <QJniObject>
#include <QCoreApplication>
#include <QColor>


// WindowManager.LayoutParams
#define FLAG_TRANSLUCENT_STATUS 0x04000000
#define FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS 0x80000000
#define APPEARANCE_LIGHT_STATUS_BARS 0x00000008
// View
#define SYSTEM_UI_FLAG_LIGHT_STATUS_BAR 0x00002000

static QJniObject getAndroidWindow()
{
	QJniObject context = QNativeInterface::QAndroidApplication::context();
	QJniObject obj = context.callObjectMethod("getWindow",
											  "()Landroid/view/Window;");
	if (QNativeInterface::QAndroidApplication::sdkVersion() <= 30)
		obj.callMethod<void>("clearFlags", "(I)V", FLAG_TRANSLUCENT_STATUS);
	obj.callMethod<void>("addFlags", "(I)V", FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
	return obj;
}

bool StatusBarPrivate::isAvailable_sys()
{
	return QNativeInterface::QAndroidApplication::sdkVersion() >= 21;
}

void StatusBarPrivate::setColor_sys(const QColor &color)
{
	if (QNativeInterface::QAndroidApplication::sdkVersion() < 21)
		return;

	QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
		QJniObject window = getAndroidWindow();
		window.callMethod<void>("setStatusBarColor", "(I)V", color.rgba());
	});
}

void StatusBarPrivate::setTheme_sys(StatusBar::Theme theme)
{
	if (QNativeInterface::QAndroidApplication::sdkVersion() < 23)
		return;

	QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
		QJniObject window = getAndroidWindow();
		if (QNativeInterface::QAndroidApplication::sdkVersion() > 30) {
			QJniObject insetController = window.callObjectMethod(
						"getInsetsController",
						"()Landroid/view/WindowInsetsController;");
			insetController.callMethod<void>(
						"setSystemBarsAppearance", "(II)V",
						(theme == StatusBar::Theme::Light ?
							 APPEARANCE_LIGHT_STATUS_BARS: 0),
						APPEARANCE_LIGHT_STATUS_BARS);
		} else {
			QJniObject view = window.callObjectMethod("getDecorView",
													  "()Landroid/view/View;");
			int visibility = view.callMethod<int>("getSystemUiVisibility",
												  "()I");
			if (theme == StatusBar::Theme::Light)
				visibility |= SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
			else
				visibility &= ~SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
			view.callMethod<void>("setSystemUiVisibility", "(I)V", visibility);

		}
	});
}

void StatusBarPrivate::setForceDarkMode_sys(bool force)
{
	QNativeInterface::QAndroidApplication::runOnAndroidMainThread(
				[=]() -> void {
		QJniObject window = getAndroidWindow();
		QJniObject view = window.callObjectMethod("getDecorView",
												  "()Landroid/view/View;");
		view.callMethod<void>("setForceDarkAllowed", "(Z)V", false);
	});
}

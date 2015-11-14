#ifdef USE_APP
#	include "VdkApp.h"
#	undef USE_APP
#	define USE_APP VdkApp App;
#else
#	define USE_APP
#endif

#ifdef USE_ARTPROVIDER
#	include "VdkArtProvider.h"
#	undef USE_ARTPROVIDER
#	define USE_ARTPROVIDER VdkArtProvider ArtProvider;
#else
#	define USE_ARTPROVIDER
#endif

#ifdef USE_AVATAR
#	include "VdkAvatar.h"
#	undef USE_AVATAR
#	define USE_AVATAR VdkAvatar Avatar;
#else
#	define USE_AVATAR
#endif

#ifdef USE_BITMAPARRAY
#	include "VdkBitmapArray.h"
#	undef USE_BITMAPARRAY
#	define USE_BITMAPARRAY VdkBitmapArray BitmapArray;
#else
#	define USE_BITMAPARRAY
#endif

#ifdef USE_BUTTON
#	include "VdkButton.h"
#	undef USE_BUTTON
#	define USE_BUTTON VdkButton Button;
#else
#	define USE_BUTTON
#endif

#ifdef USE_CAPTIONBAR
#	include "VdkCaptionBar.h"
#	undef USE_CAPTIONBAR
#	define USE_CAPTIONBAR VdkCaptionBar CaptionBar;
#else
#	define USE_CAPTIONBAR
#endif

#ifdef USE_CHECKBOX
#	include "VdkCheckBox.h"
#	undef USE_CHECKBOX
#	define USE_CHECKBOX VdkCheckBox CheckBox;
#else
#	define USE_CHECKBOX
#endif

#ifdef USE_CHOICECTRL
#	include "VdkChoiceCtrl.h"
#	undef USE_CHOICECTRL
#	define USE_CHOICECTRL VdkChoiceCtrl ChoiceCtrl;
#else
#	define USE_CHOICECTRL
#endif

#ifdef USE_CONTROL
#	include "VdkControl.h"
#	undef USE_CONTROL
#	define USE_CONTROL VdkControl Control;
#else
#	define USE_CONTROL
#endif

#ifdef USE_CTRLID
#	include "VdkCtrlId.h"
#	undef USE_CTRLID
#	define USE_CTRLID VdkCtrlId CtrlId;
#else
#	define USE_CTRLID
#endif

#ifdef USE_CTRLPARSERINFO
#	include "VdkCtrlParserInfo.h"
#	undef USE_CTRLPARSERINFO
#	define USE_CTRLPARSERINFO VdkCtrlParserInfo CtrlParserInfo;
#else
#	define USE_CTRLPARSERINFO
#endif

#ifdef USE_DC
#	include "VdkDC.h"
#	undef USE_DC
#	define USE_DC VdkDC DC;
#else
#	define USE_DC
#endif

#ifdef USE_EDIT
#	include "VdkEdit.h"
#	undef USE_EDIT
#	define USE_EDIT VdkEdit Edit;
#else
#	define USE_EDIT
#endif

#ifdef USE_SHORTCUT_CTRL
#	include "VdkShortcutCtrl.h"
#	undef USE_SHORTCUT_CTRL
#	define USE_SHORTCUT_CTRL VdkShortcutCtrl ShortcutCtrl;
#else
#	define USE_SHORTCUT_CTRL
#endif

#ifdef USE_EVENT
#	include "VdkEvent.h"
#	undef USE_EVENT
#	define USE_EVENT VdkEvent Event;
#else
#	define USE_EVENT
#endif

#ifdef USE_FILESELECTOR
#	include "VdkFileSelector.h"
#	undef USE_FILESELECTOR
#	define USE_FILESELECTOR VdkFileSelector FileSelector;
#else
#	define USE_FILESELECTOR
#endif

#ifdef USE_FONTPICKER
#	include "VdkFontPicker.h"
#	undef USE_FONTPICKER
#	define USE_FONTPICKER VdkFontPicker FontPicker;
#else
#	define USE_FONTPICKER
#endif

#ifdef USE_GRADIENTLINE
#	include "VdkGradientLine.h"
#	undef USE_GRADIENTLINE
#	define USE_GRADIENTLINE VdkGradientLine GradientLine;
#else
#	define USE_GRADIENTLINE
#endif

#ifdef USE_HOTAREA
#	include "VdkHotArea.h"
#	undef USE_HOTAREA
#	define USE_HOTAREA VdkHotArea HotArea;
#else
#	define USE_HOTAREA
#endif

#ifdef USE_HYPERLINK
#	include "VdkHyperLink.h"
#	undef USE_HYPERLINK
#	define USE_HYPERLINK VdkHyperLink HyperLink;
#else
#	define USE_HYPERLINK
#endif

#ifdef USE_LABEL
#	include "VdkLabel.h"
#	undef USE_LABEL
#	define USE_LABEL VdkLabel Label;
#else
#	define USE_LABEL
#endif

#ifdef USE_LISTBOX
#	include "VdkListBox.h"
#	undef USE_LISTBOX
#	define USE_LISTBOX VdkListBox ListBox;
#else
#	define USE_LISTBOX
#endif

#ifdef USE_LISTCTRL
#	include "VdkListCtrl.h"
#	undef USE_LISTCTRL
#	define USE_LISTCTRL VdkListCtrl ListCtrl;
#else
#	define USE_LISTCTRL
#endif

#ifdef USE_MENU
#	include "VdkMenu.h"
#	undef USE_MENU
#	define USE_MENU VdkMenu Menu;
#else
#	define USE_MENU
#endif

#ifdef USE_MENUBAR
#	include "VdkMenuBar.h"
#	undef USE_MENUBAR
#	define USE_MENUBAR VdkMenuBar MenuBar;
#else
#	define USE_MENUBAR
#endif

#ifdef USE_MENUPOPPER
#	include "VdkMenuPopper.h"
#	undef USE_MENUPOPPER
#	define USE_MENUPOPPER VdkMenuPopper MenuPopper;
#else
#	define USE_MENUPOPPER
#endif

#ifdef USE_OBJECT
#	include "VdkObject.h"
#	undef USE_OBJECT
#	define USE_OBJECT VdkObject Object;
#else
#	define USE_OBJECT
#endif

#ifdef USE_PANEL
#	include "VdkPanel.h"
#	undef USE_PANEL
#	define USE_PANEL VdkPanel Panel;
#else
#	define USE_PANEL
#endif

#ifdef USE_RICHLABEL
#	include "VdkRichLabel.h"
#	undef USE_RICHLABEL
#	define USE_RICHLABEL VdkRichLabel RichLabel;
#else
#	define USE_RICHLABEL
#endif

#ifdef USE_RICHTEXTCTRL
#	include "Optional/VdkRichTextCtrl.h"
#	undef USE_RICHTEXTCTRL
#	define USE_RICHTEXTCTRL VdkRichTextCtrl RichTextCtrl;
#else
#	define USE_RICHTEXTCTRL
#endif

#ifdef USE_SCALABLEBITMAP
#	include "VdkScalableBitmap.h"
#	undef USE_SCALABLEBITMAP
#	define USE_SCALABLEBITMAP VdkScalableBitmap ScalableBitmap;
#else
#	define USE_SCALABLEBITMAP
#endif

#ifdef USE_SCROLLBAR
#	include "VdkScrollBar.h"
#	undef USE_SCROLLBAR
#	define USE_SCROLLBAR VdkScrollBar ScrollBar;
#else
#	define USE_SCROLLBAR
#endif

#ifdef USE_SCROLLEDPANEL
#	include "VdkScrolledPanel.h"
#	undef USE_SCROLLEDPANEL
#	define USE_SCROLLEDPANEL VdkScrolledPanel ScrolledPanel;
#else
#	define USE_SCROLLEDPANEL
#endif

#ifdef USE_SCROLLEDWINDOW
#	include "VdkScrolledWindow.h"
#	undef USE_SCROLLEDWINDOW
#	define USE_SCROLLEDWINDOW VdkScrolledWindow ScrolledWindow;
#else
#	define USE_SCROLLEDWINDOW
#endif

#ifdef USE_SEARCHCTRL
#	include "VdkSearchCtrl.h"
#	undef USE_SEARCHCTRL
#	define USE_SEARCHCTRL VdkSearchCtrl SearchCtrl;
#else
#	define USE_SEARCHCTRL
#endif

#ifdef USE_SIDEPANEL
#	include "VdkSidePanel.h"
#	undef USE_SIDEPANEL
#	define USE_SIDEPANEL VdkSidePanel SidePanel;
#else
#	define USE_SIDEPANEL
#endif

#ifdef USE_SLIDER
#	include "VdkSlider.h"
#	undef USE_SLIDER
#	define USE_SLIDER VdkSlider Slider;
#else
#	define USE_SLIDER
#endif

#ifdef USE_STATICIMAGE
#	include "VdkStaticImage.h"
#	undef USE_STATICIMAGE
#	define USE_STATICIMAGE VdkStaticImage StaticImage;
#else
#	define USE_STATICIMAGE
#endif

#ifdef USE_SWITCHER
#	include "VdkSwitcher.h"
#	undef USE_SWITCHER
#	define USE_SWITCHER VdkSwitcher Switcher;
#else
#	define USE_SWITCHER
#endif

#ifdef USE_TAB
#	include "VdkTab.h"
#	undef USE_TAB
#	define USE_TAB VdkTab Tab;
#else
#	define USE_TAB
#endif

#ifdef USE_TOGGLEBUTTON
#	include "VdkToggleButton.h"
#	undef USE_TOGGLEBUTTON
#	define USE_TOGGLEBUTTON VdkToggleButton ToggleButton;
#else
#	define USE_TOGGLEBUTTON
#endif

#ifdef USE_TOOLBAR
#	include "VdkToolBar.h"
#	undef USE_TOOLBAR
#	define USE_TOOLBAR VdkToolBar ToolBar;
#else
#	define USE_TOOLBAR
#endif

#ifdef USE_UTIL
#	include "VdkUtil.h"
#	undef USE_UTIL
#	define USE_UTIL VdkUtil Util;
#else
#	define USE_UTIL
#endif

#ifdef USE_WAITABLEOBJ
#	include "VdkWaitableObj.h"
#	undef USE_WAITABLEOBJ
#	define USE_WAITABLEOBJ VdkWaitableObj WaitableObj;
#else
#	define USE_WAITABLEOBJ
#endif

#ifdef USE_WINDOW
#	include "VdkWindow.h"
#	undef USE_WINDOW
#	define USE_WINDOW VdkWindow Window;
#else
#	define USE_WINDOW
#endif

#ifdef USE_WINDOWSPLITTER
#	include "VdkWindowSplitter.h"
#	undef USE_WINDOWSPLITTER
#	define USE_WINDOWSPLITTER VdkWindowSplitter WindowSplitter;
#else
#	define USE_WINDOWSPLITTER
#endif

#ifdef USE_WORKER
#	include "VdkWorker.h"
#	undef USE_WORKER
#	define USE_WORKER VdkWorker Worker;
#else
#	define USE_WORKER
#endif

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

void Dummy()
{
	USE_APP
	USE_ARTPROVIDER
	USE_AVATAR
	USE_BITMAPARRAY
	USE_BUTTON
	USE_CAPTIONBAR
	USE_CHECKBOX
	USE_CHOICECTRL
	USE_CONTROL
	USE_CTRLID
	USE_CTRLPARSERINFO
	USE_DC
	USE_EDIT
	USE_SHORTCUT_CTRL
	USE_EVENT
	USE_FILESELECTOR
	USE_FONTPICKER
	USE_GRADIENTLINE
	USE_HOTAREA
	USE_HYPERLINK
	USE_LABEL
	USE_LISTBOX
	USE_LISTCTRL
	USE_MENU
	USE_MENUBAR
	USE_MENUPOPPER
	USE_OBJECT
	USE_PANEL
	USE_RICHLABEL
	USE_RICHTEXTCTRL
	USE_SCALABLEBITMAP
	USE_SCROLLBAR
	USE_SCROLLEDPANEL
	USE_SCROLLEDWINDOW
	USE_SEARCHCTRL
	USE_SIDEPANEL
	USE_SLIDER
	USE_STATICIMAGE
	USE_SWITCHER
	USE_TAB
	USE_TOGGLEBUTTON
	USE_TOOLBAR
	USE_UTIL
	USE_WAITABLEOBJ
	USE_WINDOW
	USE_WINDOWSPLITTER
	USE_WORKER
}

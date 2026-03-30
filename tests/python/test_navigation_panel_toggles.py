from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class NavigationPanelToggleTests(unittest.TestCase):
    def test_navigation_edge_buttons_toggle_sidebar_and_detail_panel(self) -> None:
        main_text = (REPO_ROOT / "src/app/qml/Main.qml").read_text(encoding="utf-8")
        body_text = (REPO_ROOT / "src/app/qml/view/panels/BodyLayout.qml").read_text(encoding="utf-8")
        navigation_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/NavigationBarLayout.qml"
        ).read_text(encoding="utf-8")
        properties_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/NavigationPropertiesBar.qml"
        ).read_text(encoding="utf-8")
        information_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/NavigationInformationBar.qml"
        ).read_text(encoding="utf-8")
        preference_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/NavigationPreferenceBar.qml"
        ).read_text(encoding="utf-8")
        control_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/control/NavigationApplicationControlBar.qml"
        ).read_text(encoding="utf-8")
        edit_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/edit/NavigationApplicationEditBar.qml"
        ).read_text(encoding="utf-8")
        view_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/view/NavigationApplicationViewBar.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("property bool hideSidebar: false", main_text)
        self.assertIn("property bool hideRightPanel: false", main_text)
        self.assertIn("readonly property int sidebarWidth: hideSidebar ? 0 : Math.max(minSidebarWidth, preferredSidebarWidth)", main_text)
        self.assertIn("readonly property int rightPanelWidth: hideRightPanel ? 0 : Math.max(minRightPanelWidth, preferredRightPanelWidth)", main_text)
        self.assertIn("function toggleSidebarVisibility()", main_text)
        self.assertIn("function toggleDetailPanelVisibility()", main_text)
        self.assertIn("applicationWindow.hideSidebar = !applicationWindow.hideSidebar;", main_text)
        self.assertIn("applicationWindow.hideRightPanel = !applicationWindow.hideRightPanel;", main_text)
        self.assertIn("sidebarCollapsed: applicationWindow.hideSidebar", main_text)
        self.assertIn("detailPanelCollapsed: applicationWindow.hideRightPanel", main_text)
        self.assertIn("onToggleSidebarRequested: applicationWindow.toggleSidebarVisibility()", main_text)
        self.assertIn("onToggleDetailPanelRequested: applicationWindow.toggleDetailPanelVisibility()", main_text)

        self.assertIn("signal toggleSidebarRequested", navigation_bar_text)
        self.assertIn("signal toggleDetailPanelRequested", navigation_bar_text)
        self.assertIn("sidebarCollapsed: navigationBar.sidebarCollapsed", navigation_bar_text)
        self.assertIn("detailPanelCollapsed: navigationBar.detailPanelCollapsed", navigation_bar_text)
        self.assertIn("onToggleSidebarRequested: navigationBar.toggleSidebarRequested()", navigation_bar_text)
        self.assertIn("onToggleDetailPanelRequested: navigationBar.toggleDetailPanelRequested()", navigation_bar_text)

        self.assertIn("property bool sidebarCollapsed: false", properties_text)
        self.assertIn("signal toggleSidebarRequested", properties_text)
        self.assertIn("sidebarCollapsed: propertiesBar.sidebarCollapsed", properties_text)
        self.assertIn("onToggleSidebarRequested: propertiesBar.toggleSidebarRequested()", properties_text)

        self.assertIn("signal toggleSidebarRequested", information_text)
        self.assertIn("informationBar.toggleSidebarRequested();", information_text)
        self.assertIn('informationBar.requestViewHook(informationBar.sidebarCollapsed ? "expand-sidebar" : "collapse-sidebar");', information_text)

        self.assertIn("signal toggleDetailPanelRequested", preference_text)
        self.assertIn("preferenceBar.toggleDetailPanelRequested();", preference_text)
        self.assertIn('preferenceBar.requestViewHook(preferenceBar.detailPanelCollapsed ? "expand-detail-panel" : "collapse-detail-panel");', preference_text)

        self.assertIn("signal toggleDetailPanelRequested", control_text)
        self.assertIn("detailPanelCollapsed: applicationControlBar.detailPanelCollapsed", control_text)
        self.assertIn("onToggleDetailPanelRequested: applicationControlBar.toggleDetailPanelRequested()", control_text)

        self.assertIn("signal toggleDetailPanelRequested", edit_text)
        self.assertIn("detailPanelCollapsed: applicationEditBar.detailPanelCollapsed", edit_text)
        self.assertIn("onToggleDetailPanelRequested: applicationEditBar.toggleDetailPanelRequested()", edit_text)

        self.assertIn("signal toggleDetailPanelRequested", view_text)
        self.assertIn("detailPanelCollapsed: applicationViewBar.detailPanelCollapsed", view_text)
        self.assertIn("onToggleDetailPanelRequested: applicationViewBar.toggleDetailPanelRequested()", view_text)

        self.assertIn("readonly property bool sidebarVisible: hStack.sidebarWidth > 0", body_text)
        self.assertIn("Layout.minimumWidth: hStack.sidebarVisible ? hStack.effectiveMinSidebarWidth : 0", body_text)
        self.assertIn("Layout.preferredWidth: hStack.sidebarVisible ? hStack.sidebarWidth : 0", body_text)
        self.assertIn("searchFieldVisible: true", body_text)
        self.assertIn("visible: hStack.sidebarVisible", body_text)
        self.assertIn("Layout.minimumWidth: hStack.rightVisible ? hStack.minRightPanelWidth : 0", body_text)
        self.assertIn("Layout.preferredWidth: hStack.rightVisible ? hStack.rightPanelWidth : 0", body_text)
        self.assertIn("var width = hStack.sidebarVisible ? hStack.splitterThickness : 0;", body_text)
        self.assertIn("height: Math.max(1, hStack.splitterThickness)", body_text)
        self.assertIn("color: hStack.splitterColor", body_text)

    def test_navigation_panel_toggle_contract_is_documented(self) -> None:
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertIn("left/right edge buttons", readme_text)
        self.assertIn("hierarchy sidebar", readme_text)
        self.assertIn("detail panel", readme_text)
        self.assertIn("toggle the hierarchy sidebar and detail panel", architecture_text)
        self.assertIn("preserving the stored preferred widths", architecture_text)

    def test_calendar_navigation_mounts_inline_content_surface_contract(self) -> None:
        main_text = (REPO_ROOT / "src/app/qml/Main.qml").read_text(encoding="utf-8")
        nav_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/NavigationBarLayout.qml"
        ).read_text(encoding="utf-8")
        calendar_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/NavigationCalendarBar.qml"
        ).read_text(encoding="utf-8")
        content_text = (
            REPO_ROOT / "src/app/qml/view/panels/ContentViewLayout.qml"
        ).read_text(encoding="utf-8")
        year_calendar_text = (
            REPO_ROOT / "src/app/qml/view/calendar/YearCalendarPage.qml"
        ).read_text(encoding="utf-8")
        app_main_text = (REPO_ROOT / "src/app/main.cpp").read_text(encoding="utf-8")
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertIn("signal dayCalendarRequested", nav_bar_text)
        self.assertIn("hookReason.indexOf(\"daily-calendar\") >= 0", nav_bar_text)
        self.assertIn('calendarBar.requestViewHook("open-daily-calendar")', calendar_bar_text)
        self.assertIn("signal monthCalendarRequested", nav_bar_text)
        self.assertIn("hookReason.indexOf(\"monthly-calendar\") >= 0", nav_bar_text)
        self.assertIn('calendarBar.requestViewHook("open-monthly-calendar")', calendar_bar_text)
        self.assertIn("signal weekCalendarRequested", nav_bar_text)
        self.assertIn("hookReason.indexOf(\"weekly-calendar\") >= 0", nav_bar_text)
        self.assertIn('calendarBar.requestViewHook("open-weekly-calendar")', calendar_bar_text)
        self.assertIn("signal yearCalendarRequested", nav_bar_text)
        self.assertIn("hookReason.indexOf(\"yearly-calendar\") >= 0", nav_bar_text)
        self.assertIn('calendarBar.requestViewHook("open-yearly-calendar")', calendar_bar_text)

        self.assertIn("property bool dayCalendarOverlayVisible: false", main_text)
        self.assertIn("rootDayCalendarViewModel", main_text)
        self.assertIn("onDayCalendarRequested: {", main_text)
        self.assertIn("onDayCalendarOverlayDismissRequested: applicationWindow.dayCalendarOverlayVisible = false", main_text)
        self.assertIn("property bool monthCalendarOverlayVisible: false", main_text)
        self.assertIn("rootMonthCalendarViewModel", main_text)
        self.assertIn("onMonthCalendarRequested: {", main_text)
        self.assertIn("onMonthCalendarOverlayDismissRequested: applicationWindow.monthCalendarOverlayVisible = false", main_text)
        self.assertIn("property bool weekCalendarOverlayVisible: false", main_text)
        self.assertIn("rootWeekCalendarViewModel", main_text)
        self.assertIn("onWeekCalendarRequested: {", main_text)
        self.assertIn("onWeekCalendarOverlayDismissRequested: applicationWindow.weekCalendarOverlayVisible = false", main_text)
        self.assertIn("property bool yearCalendarOverlayVisible: false", main_text)
        self.assertIn("rootYearCalendarViewModel", main_text)
        self.assertIn("onYearCalendarRequested: {", main_text)
        self.assertIn("onYearCalendarOverlayDismissRequested: applicationWindow.yearCalendarOverlayVisible = false", main_text)
        self.assertIn('import "../calendar" as CalendarView', content_text)
        self.assertIn("visible: !contentViewLayout.calendarOverlayVisible", content_text)
        self.assertIn("id: calendarContentSurface", content_text)
        self.assertIn("function onCurrentNoteIdChanged()", content_text)
        self.assertIn("contentViewLayout.requestActiveCalendarOverlayClose()", content_text)
        self.assertNotIn("id: calendarOverlay", content_text)
        self.assertNotIn("MouseArea {", content_text)
        self.assertIn("CalendarView.DayCalendarPage {", content_text)
        self.assertIn("signal dayCalendarOverlayCloseRequested", content_text)
        self.assertIn("CalendarView.MonthCalendarPage {", content_text)
        self.assertIn("signal monthCalendarOverlayCloseRequested", content_text)
        self.assertIn("CalendarView.WeekCalendarPage {", content_text)
        self.assertIn("signal weekCalendarOverlayCloseRequested", content_text)
        self.assertIn("CalendarView.YearCalendarPage {", content_text)
        self.assertIn("signal yearCalendarOverlayCloseRequested", content_text)
        self.assertIn("pragma ComponentBehavior: Bound", year_calendar_text)
        self.assertIn("required property var modelData", year_calendar_text)
        self.assertIn(
            "readonly property var optionModel: calendarSystemButton.modelData",
            year_calendar_text,
        )
        self.assertIn(
            "readonly property var dayModel: dayCell.modelData",
            year_calendar_text,
        )

        self.assertIn('setContextProperty(QStringLiteral("dayCalendarViewModel"), &dayCalendarViewModel);', app_main_text)
        self.assertIn('setContextProperty(QStringLiteral("monthCalendarViewModel"), &monthCalendarViewModel);', app_main_text)
        self.assertIn('setContextProperty(QStringLiteral("weekCalendarViewModel"), &weekCalendarViewModel);', app_main_text)
        self.assertIn('setContextProperty(QStringLiteral("yearCalendarViewModel"), &yearCalendarViewModel);', app_main_text)
        self.assertIn("daily calendar view", readme_text)
        self.assertIn("monthly calendar view", readme_text)
        self.assertIn("weekly calendar view", readme_text)
        self.assertIn("yearly calendar view", readme_text)
        self.assertIn("inline content slot", readme_text)
        self.assertIn("daily/weekly/monthly/yearly calendar actions from navigation", architecture_text)
        self.assertIn("inline content surfaces", architecture_text)


if __name__ == "__main__":
    unittest.main()

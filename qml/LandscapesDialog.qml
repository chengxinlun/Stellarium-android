import QtQuick 2.2

StelDialog {
	id: root
	title: qsTr("Landscape")
	width: rootStyle.niceMenuWidth

	property string current: stellarium.currentLandscapeName

	ListView {
		id: list
		width: 140*rootStyle.scale
		height: 400*rootStyle.scale
		delegate: StelListItem {
			text: modelData
			selected: root.current == modelData
			onClicked: stellarium.currentLandscapeName = modelData
		}
		model: stellarium.getLandscapeNames()
		clip: true
	}

	Flickable {
		anchors {
			left: list.right
			top: parent.top
			right: parent.right
			bottom: parent.bottom
			margins: rootStyle.margin
		}
		clip: true
		contentWidth: width
		contentHeight: descriptionText.height
		flickableDirection: Flickable.VerticalFlick

		Text {
			y: -20*rootStyle.scale // compensate the <h2> top margin
			id: descriptionText
			width: parent.width
			text: stellarium.currentLandscapeHtmlDescription
			wrapMode: Text.Wrap
			color: "white"
			font.pixelSize: rootStyle.fontSmallSize
		}
	}
}

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import Qt.labs.platform as Native

ApplicationWindow {
    id: win
    visible: true
    width: 1100; height: 700
    title: "OpenDHC"

    // Material style
    Material.theme: Material.Dark
    Material.accent: Material.Blue

    //
    // Reusable platform dialogs (declared once, opened from handlers)
    //
    Native.MessageDialog { id: infoDialog }

    Native.FileDialog {
        id: chdPicker
        title: "Select chdman"
        fileMode: Native.FileDialog.OpenFile
        onAccepted: {
            chd.text = file
            settings.chdmanPath = file
        }
    }

    Native.FolderDialog {
        id: outFolder
        title: "Select output folder"
        onAccepted: {
            outDir.text = folder
            settings.outputDir = folder
        }
    }

    Native.FileDialog {
        id: inputsPicker
        title: "Select input(s)"
        fileMode: Native.FileDialog.OpenFiles
        onAccepted: {
            input.text = files.join(";")
        }
    }

    Native.FolderDialog {
        id: batchFolder
        title: "Choose source folder"
        onAccepted: {
            batchSource.text = folder
        }
    }

    Native.FileDialog {
        id: saveCsvDialog
        title: "Save CSV"
        fileMode: Native.FileDialog.SaveFile
        nameFilters: [ "CSV (*.csv)" ]
        onAccepted: {
            if (!report.saveCsv(file)) {
                infoDialog.text = "Could not write CSV"
                infoDialog.open()
            }
        }
    }

    header: ToolBar {
        RowLayout { anchors.fill: parent; spacing: 12
            Label { text: "OpenDHC"; font.pixelSize: 18; Layout.margins: 8 }
            Item { Layout.fillWidth: true }
            Button {
                text: "Probe chdman"
                onClicked: {
                    const ok = runner.probeChdman()
                    infoDialog.text = ok ? "chdman detected" : "chdman not found"
                    infoDialog.open()
                }
            }
            ToolButton { text: "Settings"; onClicked: settingsPane.open() }
        }
    }

    Drawer {
        id: settingsPane; edge: Qt.RightEdge; width: 360
        ColumnLayout {
            anchors.fill: parent; spacing: 10; padding: 16
            Label { text: "Settings"; font.bold: true; font.pixelSize: 20 }

            RowLayout {
                TextField {
                    id: chd
                    text: settings.chdmanPath
                    placeholderText: "Path to chdman"
                    Layout.fillWidth: true
                    onEditingFinished: settings.chdmanPath = text
                }
                Button { text: "Browse"; onClicked: chdPicker.open() }
            }

            RowLayout {
                TextField {
                    id: outDir
                    text: settings.outputDir
                    placeholderText: "Default output folder"
                    Layout.fillWidth: true
                    onEditingFinished: settings.outputDir = text
                }
                Button { text: "Browse"; onClicked: outFolder.open() }
            }

            RowLayout {
                Label { text: "Concurrency" }
                Slider {
                    id: conc; from: 1; to: 16; stepSize: 1
                    value: settings.concurrency
                    Layout.fillWidth: true
                    onMoved: settings.concurrency = Math.round(value)
                }
                Label { text: Math.round(conc.value).toString() }
            }
        }
    }

    SplitView {
        anchors.fill: parent; orientation: Qt.Vertical

        // Composer + Batch
        Frame {
            padding: 12
            ColumnLayout {
                spacing: 8

                RowLayout {
                    Label { text: "Job Type" }
                    ComboBox { id: jobType; model: ["Create","Verify","Info","Extract"] }
                    Label { text: "Media" }
                    ComboBox { id: media; model: ["CD","DVD"] }
                    CheckBox { id: delSrc; text: "Delete source on success" }
                    CheckBox { id: keepTree; checked: true; text: "Preserve folder structure" }
                    Item { Layout.fillWidth: true }
                }

                RowLayout {
                    TextField { id: input; placeholderText: "Input file(s) (semicolon separated)"; Layout.fillWidth: true }
                    Button { text: "Browse"; onClicked: inputsPicker.open() }
                }

                RowLayout {
                    TextField { id: output; placeholderText: "Output folder / file"; text: settings.outputDir; Layout.fillWidth: true }
                    Button { text: "Browse"; onClicked: outFolder.open() }
                }

                RowLayout {
                    CheckBox { id: adv; text: "Advanced options" }
                    TextField { id: codecs; enabled: adv.checked; placeholderText: "Compression (-c), e.g. lzma,flac"; Layout.fillWidth: true }
                    SpinBox  { id: hs; enabled: adv.checked; from: 0; to: 65536; value: 0; editable: true }
                    Label    { text: "Hunk Size (-hs)"; visible: adv.checked }
                    SpinBox  { id: np; enabled: adv.checked; from: 1; to: 32; value: 4; editable: true }
                    Label    { text: "Threads (-np)"; visible: adv.checked }
                }

                RowLayout {
                    Button {
                        text: "Add to Queue"
                        onClicked: {
                            const files = input.text ? input.text.split(";") : []
                            const extra = []
                            if (adv.checked && codecs.text.length>0) { extra.push("-c", codecs.text) }
                            if (adv.checked && hs.value>0)          { extra.push("-hs", String(hs.value)) }
                            if (adv.checked && np.value>0)          { extra.push("-np", String(np.value)) }
                            for (const f of files) {
                                const id = jobModel.addJob(jobType.currentIndex, media.currentIndex, f, output.text, extra, delSrc.checked, keepTree.checked)
                                runner.enqueueSimple(id, jobType.currentIndex, media.currentIndex, f, output.text, extra, delSrc.checked, keepTree.checked)
                            }
                        }
                    }
                    Item { Layout.fillWidth: true }
                }

                Rectangle { height: 1; color: "#333"; Layout.fillWidth: true }

                // Batch from folder
                RowLayout {
                    Label { text: "Batch Source" }
                    TextField { id: batchSource; placeholderText: "Source folder"; Layout.fillWidth: true }
                    Button { text: "Folder"; onClicked: batchFolder.open() }
                    CheckBox { id: batchRecursive; text: "Recursive"; checked: true }
                    CheckBox { id: batchCD; text: "CD (.cue/.toc/.gdi)"; checked: true }
                    CheckBox { id: batchDVD; text: "DVD (.iso)"; checked: true }
                    Button {
                        text: "Add Batch"
                        onClicked: {
                            const extra = []
                            if (adv.checked && codecs.text.length>0) { extra.push("-c", codecs.text) }
                            if (adv.checked && hs.value>0)          { extra.push("-hs", String(hs.value)) }
                            if (adv.checked && np.value>0)          { extra.push("-np", String(np.value)) }
                            var inputs = scanner.findInputs(batchSource.text, batchRecursive.checked, batchCD.checked, batchDVD.checked)
                            for (let f of inputs) {
                                const ext = f.split('.').pop().toLowerCase()
                                const med = (ext === "iso") ? 1 : 0
                                const out = scanner.defaultOutputForInvokable(f, jobType.currentIndex, med, output.text, keepTree.checked)
                                const id = jobModel.addJob(jobType.currentIndex, med, f, out, extra, delSrc.checked, keepTree.checked)
                                runner.enqueueSimple(id, jobType.currentIndex, med, f, out, extra, delSrc.checked, keepTree.checked)
                            }
                        }
                    }
                    Button { text: "Final Report"; onClicked: reportDialog.open() }
                }
            }
        }

        // Queue view
        Frame {
            padding: 0
            ListView {
                anchors.fill: parent; model: jobModel; clip: true
                delegate: Frame {
                    width: ListView.view.width; padding: 12
                    background: Rectangle { color: "#121315"; radius: 8 }
                    ColumnLayout {
                        spacing: 6
                        RowLayout {
                            Label { text: "Job: " + model.id; font.bold: true }
                            Label { text: ["Create","Verify","Info","Extract"][type] + " • " + (media===0?"CD":"DVD"); color: "#8aa" }
                            Item { Layout.fillWidth: true }
                            Label { text: status; color: status==="Failed" ? "#e66" : (status==="Done" ? "#6e6" : "#ccc") }
                        }
                        ProgressBar { value: progress/100.0 }
                        TextArea { text: log; readOnly: true; wrapMode: Text.WordWrap; height: 90 }
                        RowLayout {
                            Item { Layout.fillWidth: true }
                            Button { text: "Remove"; onClicked: jobModel.removeJob(model.id) }
                        }
                    }
                }
            }
        }
    }

    // Final report dialog
    Dialog {
        id: reportDialog; modal: true; title: "Final Report"; standardButtons: Dialog.Ok
        width: Math.min(1000, parent.width*0.9); height: Math.min(600, parent.height*0.9)
        contentItem: ColumnLayout {
            spacing: 8; padding: 12
            Label {
                text:
                    "Total: "  + report.total +
                    " • OK: "  + report.ok +
                    " • Failed: " + report.failed +
                    " • Saved: "  + report.savedPct.toFixed(1) + "%"
            }
            TextArea {
                id: md; wrapMode: TextArea.NoWrap; readOnly: true
                text: report.asMarkdown()
                font.family: "monospace"; Layout.fillWidth: true; Layout.fillHeight: true
            }
            RowLayout {
                Item { Layout.fillWidth: true }
                Button {
                    text: "Copy Markdown"
                    onClicked: {
                        // Clipboard via platform API
                        let clip = Qt.createQmlObject('import Qt.labs.platform as Native; Native.Clipboard {}', win)
                        clip.text = md.text
                    }
                }
                Button { text: "Save as CSV"; onClicked: saveCsvDialog.open() }
            }
        }
    }

    // Drag & drop
    DropArea {
        anchors.fill: parent
        onDropped: function(e) {
            if (e.hasUrls) {
                input.text = e.urls.map(function(u){ return u.toString().replace("file://","") }).join(";")
                e.acceptProposedAction()
            }
        }
    }
}
// Fixed syntax error: removed invalid token

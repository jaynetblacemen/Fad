#include <QApplication>
#include <QPlainTextEdit>
#include <QIcon>
#include <QShortcut>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

// darg and drop, just that
class FedEditor : public QPlainTextEdit
{
public:
    QString currentFile = "";
    bool modified = false;

    FedEditor()
    {
        setAcceptDrops(true);
    }

    void updateTitle()
    {
        QString name = currentFile.isEmpty() ? "Untitled" : QFileInfo(currentFile).fileName();

        if(modified)
            setWindowTitle("Fed — " + name + " *");
        else
            setWindowTitle("Fed — " + name);
    }

    void loadFile(QString fileName)
    {
        QFile file(fileName);

        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            setPlainText(in.readAll());
            file.close();

            currentFile = fileName;
            modified = false;

            // This part is by ai bc i cant figure out how tf to fix curson after drag-drop and it didnt help, surely u wont DARG AND DROP
            QTextCursor c = textCursor();
            c.movePosition(QTextCursor::Start);
            setTextCursor(c);

            viewport()->update();

            updateTitle();
        }
    }

    void saveFile()
    {
        QString fileName = currentFile;

        if(fileName.isEmpty())
        {
            fileName = QFileDialog::getSaveFileName(this, "Save File", "", "All Files (*)");
            if(fileName.isEmpty())
                return;

            currentFile = fileName;
        }

        QFile file(fileName);

        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            out << toPlainText();
            file.close();

            modified = false;
            updateTitle();
        }
    }

protected:

    void dragEnterEvent(QDragEnterEvent *event) override
    {
        if(event->mimeData()->hasUrls())
            event->acceptProposedAction();
    }

    void dropEvent(QDropEvent *event) override
    {
        QList<QUrl> urls = event->mimeData()->urls();

        if(!urls.isEmpty())
        {
            QString filePath = urls.first().toLocalFile();
            loadFile(filePath);
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    FedEditor editor;

    editor.resize(900, 600);
    editor.setFrameStyle(QFrame::NoFrame);
    editor.setContextMenuPolicy(Qt::NoContextMenu);
    editor.setWindowIcon(QIcon("/home/jaynet/Developement/Fad (Text editor)/Logo.png"));

    editor.updateTitle();

    // Fow those who dont spam CTRL+S
    QObject::connect(&editor, &QPlainTextEdit::textChanged, [&](){
        editor.modified = true;
        editor.updateTitle();
    });
    QObject::connect(&editor, &QPlainTextEdit::cursorPositionChanged, [&](){
        editor.viewport()->update();
    });

    // ALT + Q function to open file.
    QShortcut *openShortcut = new QShortcut(QKeySequence("Alt+Q"), &editor);

    QObject::connect(openShortcut, &QShortcut::activated, [&](){

        QString fileName = QFileDialog::getOpenFileName(
            &editor,
            "Open File",
            "",
            "All Files (*)"
        );

        if(!fileName.isEmpty())
            editor.loadFile(fileName);

    });

    // CTRL + S
    QShortcut *saveShortcut = new QShortcut(QKeySequence("Ctrl+S"), &editor);

    QObject::connect(saveShortcut, &QShortcut::activated, [&](){
        editor.saveFile();
    });

    // Terminal argument support
    if(argc > 1)
    {
        QString fileName = argv[1];
        editor.loadFile(fileName);
    }

    editor.show();

    return app.exec();
}

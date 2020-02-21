#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListWidgetItem>
#include <QMainWindow>
#include <QThread>
#include <QMap>

typedef QHash<QString, QString> QStringMap;
Q_DECLARE_METATYPE(QStringMap)


static QString DICTIONARY_FILE = "dictionary.csv";
static int TOKENS_IN_DICTIONARY_FILE = 3; // Word, Other details, Meaning
static int MIN_WORD_LENGTH_FOR_PREDICTION = 2;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


//------------------------------------------------------------------------
// Main Window class
//------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void PopulateWordList(const QStringMap & wordDictionary);

public slots:
    void OnDictionaryLoaded(QStringMap wordDictionary);

private slots:
    void on_listWords_itemDoubleClicked(QListWidgetItem *item);

    void on_inpWord_textChanged(const QString &arg1);

    void on_btnSearch_clicked();

private:
    Ui::MainWindow *ui;
    QStringMap m_wordDictionary;
};


//------------------------------------------------------------------------
// Worker thread that loads the dictionary
//------------------------------------------------------------------------
class DictionaryLoadWorker: public QThread
{
Q_OBJECT

private:
    QStringMap m_wordDictionary;

signals:
    void DictionaryLoaded(QStringMap wordDictionary);

public:
    DictionaryLoadWorker(const QStringMap & wordDictionary) : QThread()
    {
        m_wordDictionary = wordDictionary;
    }

    void LoadDictionary();

    void run()
    {
        LoadDictionary();
        emit DictionaryLoaded(m_wordDictionary);
    }
};
#endif // MAINWINDOW_H

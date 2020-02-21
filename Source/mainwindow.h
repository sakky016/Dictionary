#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListWidgetItem>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QThread>


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
    void PrepareDictionaryModelData();
    void PopulateWordList(QStringMap * wordDictionary = nullptr);
    void ShowMeaning(const QString & word);
    void UpdateModel(QStandardItemModel* model);

public slots:
    void OnDictionaryLoaded(QStringMap * wordDictionary);

private slots:
    void on_inpWord_textChanged(const QString &arg1);

    void on_btnSearch_clicked();

    void on_listWords_doubleClicked(const QModelIndex &index);

    void on_chkUseRegex_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QStringMap *m_wordDictionary;
    bool m_bUseMatchAnywhere;
    QStandardItemModel *m_modelFilterData;
    QStandardItemModel *m_modelAllData;
    QStandardItemModel *m_activeModel;
};


//------------------------------------------------------------------------
// Worker thread that loads the dictionary
//------------------------------------------------------------------------
class DictionaryLoadWorker: public QThread
{
Q_OBJECT

private:
    QStringMap *m_wordDictionary;

signals:
    void DictionaryLoaded(QStringMap * wordDictionary);

public:
    DictionaryLoadWorker(QStringMap * wordDictionary) : QThread()
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

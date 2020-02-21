#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QRegExp>

//-------------------------------------------------------------------------------------------------------
// DictionaryLoadWorker's function which loads the dictionary in a separate thread
//-------------------------------------------------------------------------------------------------------
void DictionaryLoadWorker::LoadDictionary()
{
    QFile file(DICTIONARY_FILE);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Show an error message
        qDebug() << "Unable to open dictionary file " << DICTIONARY_FILE;
        return;
    }

    int invalidEntries = 0;
    while (!file.atEnd())
    {
        // Read an entry from the dictionary file
        QString line = file.readLine();

        // Remove any double-quotes
        line = line.remove(QChar('\"'));

        int splitIndex = line.indexOf(',');
        if (splitIndex == -1)
        {
            invalidEntries++;
            qDebug() <<  "Invalid entry [" <<invalidEntries <<"]: " << line;
        }
        else
        {
            QString word = line.left(splitIndex).toLower();
            QString meaning = line.right(line.size() - splitIndex - 1);
            m_wordDictionary[word] = meaning;
        }
    }// End of file

    qDebug() << "Words loaded: " << m_wordDictionary.size();
    if (invalidEntries)
    {
        qDebug() << "Invalid words: " << invalidEntries;
    }
}


//-------------------------------------------------------------------------------------------------------
// Main Window
//-------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<QStringMap>("QStringMap");

    m_modelFilterData = new QStandardItemModel();
    m_modelAllData = new QStandardItemModel();

    // Entire word list is the currently active model
    m_activeModel = m_modelAllData;
    m_activeModel->sort(0, Qt::AscendingOrder);

    // Create dictionary load worker and start running
    DictionaryLoadWorker *worker = new DictionaryLoadWorker(m_wordDictionary);
    connect(worker, SIGNAL(DictionaryLoaded(QStringMap)), this, SLOT(OnDictionaryLoaded(QStringMap)));
    worker->start();

    ui->statusBar->showMessage("Loading Dictionary. Please wait...");

    connect(ui->inpWord, SIGNAL(returnPressed()), this, SLOT(on_btnSearch_clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//-------------------------------------------------------------------------------------------------------
// Updates the active model data
//-------------------------------------------------------------------------------------------------------
void MainWindow::UpdateModel(QStandardItemModel* model)
{
    m_activeModel = model;
    ui->listWords->setModel(m_activeModel);
}

//-------------------------------------------------------------------------------------------------------
// Populates the word list as per the provided wordDictionary
//-------------------------------------------------------------------------------------------------------
void MainWindow::PopulateWordList(const QStringMap & wordDictionary, bool bPopulateFullData)
{
    ui->statusBar->showMessage("Populating word list...");

    if (bPopulateFullData)
    {
        UpdateModel(m_modelAllData);
    }
    else
    {
        // Clear the existing list first
        m_modelFilterData->clear();

        for(auto item = wordDictionary.begin(); item != wordDictionary.end(); item++)
        {
            QStandardItem *modelItem = new QStandardItem(item.key());
            modelItem->setFlags(modelItem->flags() &  ~Qt::ItemIsEditable);
            m_modelFilterData->appendRow(modelItem);
        }

        UpdateModel(m_modelFilterData);
    }

    int itemsPopulated = m_activeModel->rowCount();
    ui->opStatus->setText("Items: " + QString::number(itemsPopulated));
    ui->statusBar->clearMessage();
}

//-------------------------------------------------------------------------------------------------------
// Triggered when the Dictionary load has completed
//-------------------------------------------------------------------------------------------------------
void MainWindow::OnDictionaryLoaded(QStringMap wordDictionary)
{
    m_wordDictionary = wordDictionary;
    if (m_wordDictionary.empty())
    {
        ui->statusBar->showMessage("Failed to load dictionary!");
        return;
    }

    ui->statusBar->showMessage("Dictionary loaded with " + QString::number(m_wordDictionary.size()) + " words.", 2);
    PopulateWordList(m_wordDictionary);

    ui->inpWord->setFocus();
}


//-------------------------------------------------------------------------------------------------------
// Triggered whenever the text in the input field changes.
//-------------------------------------------------------------------------------------------------------
void MainWindow::on_inpWord_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
    {
        // Show entire entire word list if not already showing
        PopulateWordList(m_wordDictionary);
    }
    else if (arg1.size() > MIN_WORD_LENGTH_FOR_PREDICTION)
    {
        QRegExp regexSearch(arg1.toLower());
        QString item;
        QStringMap searchedWordsDict;
        for (auto item = m_wordDictionary.begin(); item != m_wordDictionary.end(); item++)
        {
            QString word = item.key();
            if (word.contains(regexSearch))
            {
                // Add this word to the filtered map
                searchedWordsDict[word] = m_wordDictionary[word];
            }
        }

        // Show in word list pane
        PopulateWordList(searchedWordsDict);
    }
}

//-------------------------------------------------------------------------------------------------------
// Displays word and meaning in the Right pane
//-------------------------------------------------------------------------------------------------------
void MainWindow::ShowMeaning(const QString & word)
{
    if (m_wordDictionary.contains(word))
    {
        QString meaning = m_wordDictionary[word];
        ui->txtMeaning->setHtml("<B>" + word + "</B><BR><BR>" + meaning);
    }
    else
    {
        ui->txtMeaning->setHtml("<FONT COLOR=RED>[ " + word + " ] not found!</FONT>");
    }
}

//-------------------------------------------------------------------------------------------------------
// Triggered when "Search" button is pressed
//-------------------------------------------------------------------------------------------------------
void MainWindow::on_btnSearch_clicked()
{
    QString word = ui->inpWord->text().toLower();
    if (word.isEmpty())
    {
        ui->statusBar->showMessage("Nothing to search");
        return;
    }

    ShowMeaning(word);
    ui->inpWord->setFocus();
}

//-------------------------------------------------------------------------------------------------------
// Triggered when a word is double clicked from the left pane
//-------------------------------------------------------------------------------------------------------
void MainWindow::on_listWords_doubleClicked(const QModelIndex &index)
{
    QStandardItem *item = m_activeModel->itemFromIndex(index);
    QString word = item->text();
    ShowMeaning(word);
}

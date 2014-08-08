#include <QCoreApplication>
#include <QFile>
#include <iostream>
#include <QByteArray>

#define EXPORT_DIR    "/Users/joe/Downloads/export/"
#define READ_BUFFER_SIZE 1024*4

QString currentTitle;
int exportIndex = 0;

void saveExportFile(const QByteArray& aData)
{
    QString outputFileName = EXPORT_DIR;
    outputFileName += QString::number(exportIndex++);
    outputFileName += "-";
    outputFileName += currentTitle;
    QFile outputFile(outputFileName);

    if (!outputFile.open(QIODevice::WriteOnly))
        return;
    outputFile.write(aData.data(),aData.size());

    outputFile.close();
}



void saveToFile(const char* aFilename, const char* data, size_t length)
{
    QString outputFileName = EXPORT_DIR;
    outputFileName += aFilename;
    QFile outputFile(outputFileName);

    if (!outputFile.open(QIODevice::WriteOnly))
        return;

    outputFile.write(data,length);

    outputFile.close();
}

enum BLOCK_PROC_STATE
{
    STATE_NORMAL,
    STATE_IN_PAGE,
    STATE_IN_TITLE,
    STATE_IN_TEXT,
    STATE_IN_TITLE_TOKEN,
    STATE_IN_TEXT_TOKEN,
};
BLOCK_PROC_STATE currentProcState = STATE_NORMAL;

QByteArray &getTokenBuffer()
{
    static QByteArray globalTokenBuffer;
    return globalTokenBuffer;
}


QByteArray &getTextBuffer()
{
    static QByteArray globalTextBuffer;
    return globalTextBuffer;
}

void procressDataBlock(const char* data, size_t length)
{
    //here we only need to handle following token here:
    //<page>
    //<title>
    //<text>

    //also maybe need to filter out some uncessary : translate block
    //[[xxx|yyy]],we only need keey the xxx part

    //need remove: reference & style
    //{{xxxxxx}}
    //:{xxxxxxx}

    QByteArray &tokenBuffer = getTokenBuffer();
    QByteArray &textBuffer = getTextBuffer();

    //hard coded filtter
    for(size_t pos = 0; pos<length; pos++)
    {
        //std::cout << data[pos] << std::endl;
        switch(currentProcState)
        {
        case STATE_NORMAL:
        case STATE_IN_PAGE:
            if(data[pos] == '<')
            {
                //std::cout << "token start" << std::endl;
                if(!tokenBuffer.isEmpty())
                {
                    tokenBuffer.clear();
                }

                tokenBuffer.push_back(data[pos]);
            }
            else if(data[pos] == '>')
            {
                //std::cout << "token end" << std::endl;

                /*
                std::cout << "buffer size: " << tokenBuffer.size() << std::endl;
                for(int idx=0;idx<tokenBuffer.size();idx++)
                {
                    std::cout << tokenBuffer[idx];
                }
                std::cout << std::endl;
                */
                //check the xml token here
                if(tokenBuffer.size() == 5)
                {
                    //page or text
                    if(tokenBuffer[0] == '<'
                            && tokenBuffer[1] == 'p'
                            && tokenBuffer[2] == 'a'
                            && tokenBuffer[3] == 'g'
                            && tokenBuffer[4] == 'e')
                    {
                        //page start
                        currentProcState = STATE_IN_PAGE;
                        std::cout << "page start" << std::endl;
                    }
                    else if(tokenBuffer[0] == '<'
                            && tokenBuffer[1] == 't'
                            && tokenBuffer[2] == 'e'
                            && tokenBuffer[3] == 'x'
                            && tokenBuffer[4] == 't')
                    {
                        //text
                        currentProcState = STATE_IN_TEXT;
                        std::cout << "text start" << std::endl;

                        //prepare for text data
                        textBuffer.clear();
                    }
                    else
                    {
                        //std::cout << "other element" << std::endl;
                        //other element
                        currentProcState = STATE_NORMAL;
                    }

                }
                else if(tokenBuffer.size() == 6)
                {
                    //title or /page or /text
                    if(tokenBuffer[0] == '<'
                            && tokenBuffer[1] == 't'
                            && tokenBuffer[2] == 'i'
                            && tokenBuffer[3] == 't'
                            && tokenBuffer[4] == 'l'
                            && tokenBuffer[5] == 'e')
                    {
                        //page start
                        currentProcState = STATE_IN_TITLE;
                        std::cout << "title start" << std::endl;

                        //prepare title buffer
                        textBuffer.clear();
                    }
                    else if(tokenBuffer[0] == '<'
                            && tokenBuffer[1] == '/'
                            && tokenBuffer[2] == 'p'
                            && tokenBuffer[3] == 'a'
                            && tokenBuffer[4] == 'g'
                            && tokenBuffer[5] == 'e')
                    {
                        std::cout << "page end" << std::endl;
                        //text
                        currentProcState = STATE_NORMAL;
                    }
                    else if(tokenBuffer[0] == '<'
                            && tokenBuffer[1] == '/'
                            && tokenBuffer[2] == 't'
                            && tokenBuffer[3] == 'e'
                            && tokenBuffer[4] == 'x'
                            && tokenBuffer[5] == 't')
                    {
                        //text
                        currentProcState = STATE_IN_PAGE;

                        //store text data
                        std::cout << "text end" << std::endl;
                    }
                    else
                    {
                        //std::cout << "other element" << std::endl;
                        //other element
                        currentProcState = STATE_NORMAL;
                    }

                }
                else if(tokenBuffer.size() == 7)
                {
                    // /title
                    if(tokenBuffer[0] == '<'
                            && tokenBuffer[1] == '/'
                            && tokenBuffer[2] == 't'
                            && tokenBuffer[3] == 'i'
                            && tokenBuffer[4] == 't'
                            && tokenBuffer[5] == 'l'
                            && tokenBuffer[6] == 'e')
                    {
                        //text
                        currentProcState = STATE_IN_PAGE;

                        std::cout << "title end" << std::endl;
                    }
                    else
                    {
                        //std::cout << "other element" << std::endl;
                        //other element
                        currentProcState = STATE_NORMAL;
                    }
                }
                else if(tokenBuffer.size() > 7)
                {
                    //maybe text
                    if(tokenBuffer[0] == '<'
                            && tokenBuffer[1] == 't'
                            && tokenBuffer[2] == 'e'
                            && tokenBuffer[3] == 'x'
                            && tokenBuffer[4] == 't')
                    {
                        //text
                        currentProcState = STATE_IN_TEXT;
                        std::cout << "text start" << std::endl;

                        //prepare for text data
                        textBuffer.clear();
                    }
                    else
                    {
                        //std::cout << "other element" << std::endl;
                        //other element
                        currentProcState = STATE_NORMAL;
                    }
                }
                else
                {
                    //std::cout << "other element" << std::endl;
                    currentProcState = STATE_NORMAL;
                }
            }
            else
            {
                tokenBuffer.push_back(data[pos]);
            }
            break;

        case STATE_IN_TITLE:
            if(data[pos] != '<')
            {
                textBuffer.push_back(data[pos]);
            }
            else if(data[pos] == '<')
            {
                //std::cout << "token start" << std::endl;
                if(!tokenBuffer.isEmpty())
                {
                    tokenBuffer.clear();
                }

                tokenBuffer.push_back(data[pos]);

                currentProcState = STATE_IN_TITLE_TOKEN;
            }
            break;

        case STATE_IN_TITLE_TOKEN:
            if(data[pos] != '>')
            {
                if(data[pos] == '<')
                {
                    //another element start, save the token buffer data to title
                    for(int idx=0;idx<tokenBuffer.size();idx++)
                    {
                        textBuffer.push_back(tokenBuffer[idx]);
                    }
                    tokenBuffer.clear();
                    tokenBuffer.push_back(data[pos]);
                }
                else
                {
                    tokenBuffer.push_back(data[pos]);
                }
            }
            else if(data[pos] == '>')
            {
                //std::cout << "token end" << std::endl;
                if(tokenBuffer.size() == 7)
                {
                    // /title
                    if(tokenBuffer[0] == '<'
                            && tokenBuffer[1] == '/'
                            && tokenBuffer[2] == 't'
                            && tokenBuffer[3] == 'i'
                            && tokenBuffer[4] == 't'
                            && tokenBuffer[5] == 'l'
                            && tokenBuffer[6] == 'e')
                    {
                        //text
                        currentProcState = STATE_IN_PAGE;

                        currentTitle = QString::fromUtf8(textBuffer);
                        std::cout << "title end" << std::endl;
                    }
                    else
                    {
                        //std::cout << "other element" << std::endl;
                        //other element
                        currentProcState = STATE_IN_TITLE;
                    }
                }
                else
                {
                    //std::cout << "other element" << std::endl;
                    currentProcState = STATE_IN_TITLE;
                }
            }
            break;

        case STATE_IN_TEXT:
            if(data[pos] != '<')
            {
                textBuffer.push_back(data[pos]);
            }
            else if(data[pos] == '<')
            {
                //std::cout << "token start" << std::endl;
                if(!tokenBuffer.isEmpty())
                {
                    tokenBuffer.clear();
                }

                tokenBuffer.push_back(data[pos]);

                currentProcState = STATE_IN_TEXT_TOKEN;
            }
            break;

        case STATE_IN_TEXT_TOKEN:
            if(data[pos] != '>')
            {
                if(data[pos] == '<')
                {
                    //another element start, save the token buffer data to title
                    for(int idx=0;idx<tokenBuffer.size();idx++)
                    {
                        textBuffer.push_back(tokenBuffer[idx]);
                    }
                    tokenBuffer.clear();
                    tokenBuffer.push_back(data[pos]);
                }
                else
                {
                    tokenBuffer.push_back(data[pos]);
                }
            }
            else if(data[pos] == '>')
            {
                //std::cout << "token end" << std::endl;
                if(tokenBuffer.size() == 6)
                {
                    // /title
                    if(tokenBuffer[0] == '<'
                            && tokenBuffer[1] == '/'
                            && tokenBuffer[2] == 't'
                            && tokenBuffer[3] == 'e'
                            && tokenBuffer[4] == 'x'
                            && tokenBuffer[5] == 't')
                    {
                        //text
                        currentProcState = STATE_IN_PAGE;

                        saveExportFile(textBuffer);
                        std::cout << "text end" << std::endl;
                    }
                    else
                    {
                        //std::cout << "other element" << std::endl;
                        //other element
                        currentProcState = STATE_IN_TEXT;
                    }
                }
                else
                {
                    //std::cout << "other element" << std::endl;
                    currentProcState = STATE_IN_TEXT;
                }
            }
            break;
        }
    }


}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    const char* wikiDumpFileName = "/Users/joe/Downloads/zhwiki-20140804-pages-articles.xml";
    const char* outputDir = "/Users/joe/Downloads/export";

    QFile wikiDumpFile(wikiDumpFileName);
    if (!wikiDumpFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;


    //open the file and procress it
    char* readBuffer = new char[READ_BUFFER_SIZE];
    size_t readLength = wikiDumpFile.read(readBuffer,READ_BUFFER_SIZE);
    while(readLength > 0)
    {
        procressDataBlock(readBuffer, readLength);
        readLength = wikiDumpFile.read(readBuffer,READ_BUFFER_SIZE);
    }


    delete[] readBuffer;
    wikiDumpFile.close();


    std::cout << "DONE" << std::endl;
    return a.exec();
}

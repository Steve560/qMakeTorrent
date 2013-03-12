#include "createtorrent.h"

#include <boost/version.hpp>
#if BOOST_VERSION >= 104800
#define BOOST_ASIO_DYN_LINK
#endif

#include <libtorrent/version.hpp>
#if LIBTORRENT_VERSION_MINOR < 16
#define BOOST_FILESYSTEM_VERSION 2
#endif

#include <fstream>
#include <boost/filesystem/path.hpp>
#include <boost/bind.hpp>

#include <libtorrent/create_torrent.hpp>

#include <QtGui>

CreateTorrent::CreateTorrent(QObject *parent) :
    QThread(parent)
{
}

void CreateTorrent::makeTorrentFiles(QString source, QString outputLocation, bool isBatch, QString announceUrls, QString webSeeds, QString comment, QString creator, int pieceSizeIndex, bool isPrivate) {
    this->source = source;
    this->outputLocation = outputLocation;
    this->isBatch = isBatch;
    this->announceUrls = announceUrls;
    this->webSeeds = webSeeds;
    this->comment = comment;
    this->creator = creator;
    if(pieceSizeIndex == 0)
        this->pieceSize = 0;
    else
        this->pieceSize = 1024 * (2 << (pieceSizeIndex + 2));
    this->isPrivate = isPrivate;
    start();

}

// Courtesy of qBitTorrent
#if LIBTORRENT_VERSION_MINOR >= 16
bool file_filter(std::string const& f)
{
        if (libtorrent::filename(f)[0] == '.') return false;
        return true;
}
#else
bool file_filter(boost::filesystem::path const& filename)
{
  if (filename.leaf()[0] == '.') return false;
  return true;
}
#endif

void doProgressUpdate(int i, int total, CreateTorrent *parent) {
    parent->sendProgressSignal((int) (i * 100 / (float) total));
}

void CreateTorrent::sendProgressSignal(int i) {
    emit(updateProgress(i));
}

void CreateTorrent::run() {

    using namespace libtorrent;

    QStringList inputList;
    if(!this->isBatch)
        inputList.append(this->source);
    else {
        QDirIterator iit(this->source);
        while(iit.hasNext()) {
            QString fn = iit.next();
            if(file_filter(fn.toUtf8().constData()))
                inputList.append(iit.next());
        }
    }

    QStringListIterator inputListIterator(inputList);
    while(inputListIterator.hasNext()) {
        QString input = inputListIterator.next();
        QFileInfo inputInfo(input);
        if(inputInfo.isDir()) {
            QDirIterator iit(input, QDirIterator::Subdirectories);
            while(iit.hasNext()) {
                QString fn = iit.next();
                if(file_filter(fn.toUtf8().constData()))
                    emit(logStatusMessage(fn));
            }
        }

        QString outputFilename;
        if(!this->isBatch)
            outputFilename = this->outputLocation;
        else
            outputFilename = QDir(this->outputLocation).absoluteFilePath(QDir(input).dirName() + ".torrent");

        file_storage fs;
        add_files(fs, input.toUtf8().constData(), file_filter);
        create_torrent torrent(fs);
        fs.piece_size(this->pieceSize);
        this->pieceCount = torrent.num_pieces();

        QStringListIterator webSeedList(this->webSeeds.split("\n"));
        while(webSeedList.hasNext())
            torrent.add_url_seed(webSeedList.next().trimmed().toStdString());

        int tier = 0;
        QStringListIterator trackerList(this->announceUrls.split("\n"));
        while(trackerList.hasNext()) {
            torrent.add_tracker(trackerList.next().trimmed().toStdString(), tier);
            tier++;
        }

        torrent.set_priv(this->isPrivate);
        torrent.set_comment(this->comment.toUtf8().constData());
        torrent.set_creator(this->creator.toUtf8().constData());
        QFileInfo pSource(input);
        set_piece_hashes(torrent, pSource.dir().path().toUtf8().constData(), boost::bind<void>(&doProgressUpdate, _1, torrent.num_pieces(), this));
        std::ofstream outputFile(outputFilename.toUtf8().constData(), std::ios_base::binary);
        bencode(std::ostream_iterator<char>(outputFile), torrent.generate());
        outputFile.close();
        emit(updateProgress(100));
        emit(logStatusMessage(QString("Created %1").arg(outputFilename)));
    }

}



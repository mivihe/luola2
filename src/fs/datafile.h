//
// This file is part of Luola2.
// Copyright (C) 2012 Calle Laakkonen
//
// Luola2 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Luola2 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Luola2.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef LUOLA_DATAFILE_H
#define LUOLA_DATAFILE_H

#include <memory>
#include <string>

#include <boost/iostreams/stream.hpp>

using std::shared_ptr;
using std::string;

namespace fs {

class DataFileImpl;
class DataFile;
class DataSourceImpl;

/**
 * Boost iostream source for reading data files
 *
 * Use DataStream to actually read a file.
 * This class defines a Source for reading a file from a data file
 * archive.
 *
 * Note. Due to limitations of certain backends (minizip), only one
 * DataSource per DataFile can be active at the same time!
 *
 * Call `isError()` after opening to check if the file was opened
 * properly.
 */
class DataSource : public boost::iostreams::source {
    public:
        /**
         * \brief Construct a data source
         * \param data the data file archive
         * \param resource data file name to read
         */
        DataSource(DataFile &data, const string& resource);

        std::streamsize read(char *s, std::streamsize n);

        //! Was there an error opening or reading the file?
        bool isError() const;

        //! Get the error message
        string errorString() const;

    private:
        shared_ptr<DataSourceImpl> p_;
};

typedef boost::iostreams::stream<DataSource> DataStream;

/**
 * A data file reader.
 *
 * This class is for reading data files from an archive.
 * The following archive types are supported:
 * 
 * - Directories
 * - Zip files
 *
 * Path::findDataFile is used to find the data file or directory.
 *
 * Call `isError()` after opening to see if the archive was
 * opened properly.
 *
 * To read the actual files, construct a DataStream, like so:
 *
 *     DataStream myFile(mysource, "myfilename.png");
 */
class DataFile {
    friend class DataSource;
    public:
        /**
         * Construct a data file archive loader
         *
         * \param name (relative) path name of the archive
         */
        DataFile(const string& name);

        /**
         * Get the name of the datafile
         *
         * @return file name
         */
        string name() const;

        /**
         * Was there an error opening the file?
         *
         * \return true if data file/directory couldn't be opened
         */
        bool isError() const;

        /**
         * Return the error message
         * The return value is valid only when isError() == true.
         *
         * \return error message.
         */
        string errorString() const;

    private:
        shared_ptr<DataFileImpl> p_;
};

}

#endif


/**
 * @file    sff_parser.hpp
 * @author  Mariya Fomkina
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * @section DESCRIPTION
 *
 * SamBamParser is the parser stream that reads data from .sam and
 * .bam files.
 */

#ifndef COMMON_IO_SFFPARSER_HPP
#define COMMON_IO_SFFPARSER_HPP

#include <zlib.h>
#include <string>
#include <cassert>
#include <io_lib/sff.h>
#include "common/io/single_read.hpp"
#include "common/io/parser.hpp"
#include "common/sequence/quality.hpp"
#include "common/sequence/nucl.hpp"

namespace io {

class SffParser : public Parser {
 public:
  /*
   * Default constructor.
   * 
   * @param filename The name of the file to be opened.
   * @param offset The offset of the read quality.
   */
  SffParser(const std::string& filename,
         int offset = SingleRead::PHRED_OFFSET)
      :Parser(filename, offset), read_(NULL) {
    open();
  }

  /* 
   * Default destructor.
   */
  /* virtual */ ~SffParser() {
    close();
  }

  /*
   * Read SingleRead from stream.
   *
   * @param read The SingleRead that will store read data.
   *
   * @return Reference to this stream.
   */
  /* virtual */ SffParser& operator>>(SingleRead& read) {
    if (!is_open_ || eof_) {
      return *this;
    }
    read.SetName(read_.name());
    read.SetQuality(read_.GetQualityString());
    read.SetSequence(read_.GetSequenceString());
    ReadAhead();
    return *this;
  }

  /*
   * Close the stream.
   */
  /* virtual */ void close() {
    if (is_open_) {
      free_sff_common_header(&h_);
      is_open_ = false;
      eof_ = true;
    }
  }

 private:
  sff_common_header h_;
  sff_read_header rh_;
  sff_read_data rd_;
  FILE *sff_fp_;
  int num_of_reads_;
  int cnt_;
  SingleRead read_;

int 

  /*
   * Open a stream.
   */
  /* virtual */ void open() {
    sff_fp_ = fopen(filename_.c.str(), "r");
    if (sff_fp_ == NULL) {
      eof_ = true;
      is_open_ = false;
    } else {
      read_sff_common_header(sff_fp_, &h_);
      num_of_reads_ = (int) h_.nreads;
      cnt_ = 0;
      eof_ = false;
      is_open_ = true;
      ReadAhead();
    }
  }

  /* 
   * Read next SingleRead from file.
   */
  void ReadAhead() {
    assert(is_open_);
    assert(!eof_);
    read_sff_read_header(sff_fp_, &rh_);
    read_sff_read_data(sff_fp_, &rd_, h_.flow_len, rh_.nbases);
    ++cnt_;
    if (cnt_ == num_of_reads_) {
      eof_ true;
    }
    // fill fields in sff single read
  }

  /*
   * Hidden copy constructor.
   */
  SffParser(const SffParser& parser);
  /*
   * Hidden assign operator.
   */
  void operator=(const SffParser& parser);
};

}

#endif /* COMMON_IO_SFFPARSER_HPP */

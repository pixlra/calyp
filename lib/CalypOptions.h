/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2014, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file     CalypOptions.h
 * \ingroup	 CalypLibGrp
 * \brief    Options handler
 */

#ifndef __CALYPOPTIONS_H__
#define __CALYPOPTIONS_H__

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * \class    CalypOptions
 * \ingroup  CalypLibGrp
 * \brief    Main class to store options
 */
class CalypOptions
{
public:
  /**
   * Virtual base class for storing information relating to a
   * specific option This base class describes common elements.
   * Type specific information should be stored in a derived class.
   */
  class OptionBase
  {
  public:
    OptionBase( std::string name, std::string desc, std::string def )
        : opt_string{ std::move( name ) }, opt_desc{ std::move( desc ) }, opt_defaults{ std::move( def ) } {}
    OptionBase( OptionBase&& other ) noexcept = delete;
    OptionBase& operator=( OptionBase&& other ) noexcept = delete;
    OptionBase( const OptionBase& other ) = delete;
    OptionBase& operator=( const OptionBase& other ) = delete;
    virtual ~OptionBase() = default;
    /* parse argument arg, to obtain a value for the option */
    virtual void parse( const std::string& arg ) = 0;
    auto count() const -> std::size_t { return arg_count; }
    auto isBinary() const -> bool { return is_binary; }
    std::string opt_string;
    std::string opt_desc;
    std::string opt_defaults;

  protected:
    std::size_t arg_count{ 0 };
    bool is_binary{ false };
  };
  struct Option
  {
    std::list<std::string> opt_long;
    std::list<std::string> opt_short;
    std::unique_ptr<OptionBase> base_opt{ nullptr };
  };

  CalypOptions() = default;
  CalypOptions( std::string name );

  int parse( unsigned int argc, char* argv[] );  // NOLINT
  void parse( std::vector<std::string> args_array );

  std::list<const char*>& getUnhandledArgs() { return m_aUnhandledArgs; }
  void doHelp( std::ostream& out, unsigned columns = 80 );

  CalypOptions::OptionBase* operator[]( const std::string& optName );
  CalypOptions::OptionBase* getOption( const std::string& optName );

  bool hasOpt( const std::string& optName );

  auto getOptionList() -> const std::list<std::unique_ptr<Option>>& { return opt_list; }
  CalypOptions& addOptions();

  /**
   * Add option described by name to the parent Options list,
   *   with storage for the option's value
   *   with default_val as the default value
   *   with desc as an optional help description
   */
  CalypOptions& operator()( const std::string& name, const std::string& desc );
  CalypOptions& addOption( const std::string& name, const std::string& desc );

  /**
   * Add option described by name to the parent Options list,
   *   with storage for the option's value
   *   with default_val as the default value
   *   description and range along with default value
   */
  template <typename T>
  CalypOptions& operator()( const std::string& name, T& storage, const std::string& desc );
  template <typename T>
  CalypOptions& addOption( const std::string& name, T& storage, const std::string& desc );

  template <typename T>
  CalypOptions& operator()( const std::string& name, T& storage, const std::string& desc, const std::string& defaults );
  template <typename T>
  CalypOptions& addOption( const std::string& name, T& storage, const std::string& desc, const std::string& defaults );

  bool checkListingOpts();

private:
  void addOptionInternal( std::unique_ptr<OptionBase> opt );

private:
  std::map<std::string, std::list<Option*>> opt_long_map;
  std::map<std::string, std::list<Option*>> opt_short_map;

  std::list<std::unique_ptr<Option>> opt_list;

  std::string m_cOptionGroupName{ "" };
  bool m_bAllowUnkonw{ true };
  std::list<const char*> m_aUnhandledArgs;

  bool storePair( bool allow_long, bool allow_short, const std::string& name, const std::string& value );
  bool storePair( const std::string& name, const std::string& value );
  unsigned int parseLONG( unsigned int argc, char* argv[] );  // NOLINT
  unsigned int parseLONG( const std::string& arg );
  unsigned int parseSHORT( unsigned int argc, char* argv[] );          // NOLINT
  std::list<const char*> scanArgv( unsigned int argc, char* argv[] );  // NOLINT
};

#endif  // __CALYPOPTIONS_H__

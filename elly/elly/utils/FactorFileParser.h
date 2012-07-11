//
//  FactorFileParser.h
//  elly
//
//  Created by Ce Zhang on 7/8/12.
//  Copyright (c) 2012 University of Wisconsin-Madison. All rights reserved.
//

#ifndef elly_FactorFileParser_h
#define elly_FactorFileParser_h

#include <vector>
#include "Common.h"

#include "../dstruct/InFileCorrelationRelation.h"

#include <fstream>

//#include "/usr/local/include/boost/program_options.hpp"
//namespace po = boost::program_options;

    //////!!!!!!!! TODO: so ugly !!!!!!!!!!!!!!
#include "/usr/local/include/boost/property_tree/ptree.hpp"
#include "/usr/local/include/boost/property_tree/ini_parser.hpp"
#include "/usr/local/include/boost/foreach.hpp"

namespace elly{
    namespace utils{
    
        class FactorFileParser{
        public:
            std::string folder_name;
            std::string catelog;
            
            elly::utils::Config config;
        
            std::vector<mia::elly::dstruct::InFileCorrelationRelation> crs;
            
            FactorFileParser(std::string _folder_name, elly::utils::Config& _config){
                folder_name = _folder_name;
                catelog = folder_name + "/catelog.cfg"; 
                config = _config;
            }
            
            void parse(){
                
                elly::utils::log() << ">> Parsing factors from " << catelog << std::endl;
                
                boost::property_tree::ptree pt;
                boost::property_tree::ini_parser::read_ini(catelog.c_str(), pt);
                
                boost::property_tree::ptree::const_iterator end = pt.end();
                
                std::string itype = "";
                
                for(boost::property_tree::ptree::const_iterator it = pt.begin();
                    it != end; ++it){
                    
                    if(it->first.compare("type") == 0 &&
                      it->second.data().compare("file") == 0){
                        std::cout << "  | Input type: In-file factor graph." << std::endl;
                        itype = "file";
                        break;
                    }
                    
                    if(it->first.compare("type") == 0 &&
                       it->second.data().compare("mln") == 0){
                        std::cout << "  | Input type: Markov Logic Network." << std::endl;
                        itype = "mln";
                        break;
                    }
                    
                    if(it->first.compare("type") == 0){
                        elly::utils::logerr() << "ERROR: not parsable type in config file.";
                        throw std::exception();
                    }
                    
                }
                
                
                if(itype.compare("mln") == 0){
                    
                    std::string evid = "";
                    std::string mln = "";
                    std::string query = "";
                    std::string queryfile = "";
                    
                    std::string java = "";
                    std::string path_to_tuffy = "";
                    std::string tuffy_config = "";
                    std::string other_config = "";
                    
                    for(boost::property_tree::ptree::const_iterator it = pt.begin();
                        it != end; ++it){

                        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,
                                      pt.get_child(it->first)){
                            
                            
                            if(it->first.compare("mln") == 0){
                                mln = mln + " " + v.second.data();
                            }
                            
                            if(it->first.compare("evid") == 0){
                                evid = evid + " " + v.second.data();
                            }
                            
                            if(it->first.compare("queryfile") == 0){
                                queryfile = queryfile + " "+ v.second.data();
                            }
                            
                            if(it->first.compare("query") == 0){
                                query = query + " " + v.second.data();
                            }
                            
                            if(it->first.compare("option") == 0 &&
                               v.first.compare("java") == 0){
                                java = v.second.data();
                            }
                            
                            if(it->first.compare("option") == 0 &&
                               v.first.compare("tuffy") == 0){
                                path_to_tuffy = v.second.data();
                            }
                            
                            if(it->first.compare("option") == 0 &&
                               v.first.compare("config") == 0){
                                tuffy_config = v.second.data();
                            }
                            
                            if(it->first.compare("option") == 0 &&
                               v.first.compare("other") == 0){
                                other_config = v.second.data();
                            }
                            
                        }
                        
                    }
                    
                    if(java.compare("") == 0 ||
                       path_to_tuffy.compare("") == 0 ||
                       mln.compare("") == 0 ||
                       evid.compare("") == 0){
                        elly::utils::logerr() << "ERROR: empty java/path_to_tuffy/mln" << std::endl;
                        throw std::exception();
                    }
                                        
                    elly::utils::log() << "  | MLN         = " << mln << std::endl;
                    elly::utils::log() << "  | Evid        = " << evid << std::endl;
                    elly::utils::log() << "  | Query       = " << query << std::endl;
                    elly::utils::log() << "  | QueryFile   = " << queryfile << std::endl;
                    elly::utils::log() << "  | Java        = " << java << std::endl;
                    elly::utils::log() << "  | Tuffy       = " << path_to_tuffy << std::endl;
                    elly::utils::log() << "  | TuffyConfig = " << tuffy_config << std::endl;
                    elly::utils::log() << "  | OtherConfig = " << other_config << std::endl;
                    
                    std::string cmd = "";
                    
                    cmd = java + " -jar " + path_to_tuffy + " -i " + mln + " -e " + evid;
                    
                    if(query.compare("")!=0){
                        cmd += " -q " + query;
                    }
                    if(queryfile.compare("")!=0){
                        cmd += " -queryFile " + queryfile;
                    }
                    if(tuffy_config.compare("")!=0){
                        cmd += " -conf " + tuffy_config;
                    }
                    if(other_config.compare("")!=0){
                        cmd += other_config;
                    }
                    
                    cmd += " -o " + config.rt_workdir + "/tmp_tuffy_dummy_output";
                    cmd += " -keepData -marginal -mcsatSamples 0 -verbose 3";
                    
                    std::string tuffy_log = config.rt_workdir + "/log_tuffy.txt";
                    std::string tuffy_error = config.rt_workdir + "/error_tuffy.txt";
                    
                    elly::utils::log() << ">> Executing Tuffy using command: " << cmd << std::endl;
                    elly::utils::log() << ">> Tuffy is logged at: " << tuffy_log << std::endl;
                    elly::utils::log() << ">> Tuffy error is logged at: " << tuffy_error << std::endl;
                    
                    system(((std::string)(cmd + " > " + tuffy_log + " 2> " + tuffy_error)).c_str());
                    
                }
                
                if(itype.compare("file") == 0){
                
                
                    for(boost::property_tree::ptree::const_iterator it = pt.begin();
                        it != end; ++it){
                        
                        if(it->first.compare("type") == 0){
                            continue;
                        }
                    
                        mia::elly::dstruct::InFileCorrelationRelation cr;
                        cr.factor_name = it->first;
                    
                        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,
                                  pt.get_child(it->first)){
                        
                            if(v.first.compare("fid") == 0){
                                cr.function_id = atoi(v.second.data().c_str());
                            }
                        
                            if(v.first.compare("type") == 0){
                                cr.filetype = v.second.data();
                            }
                        
                            if(v.first.compare("file") == 0){
                                cr.filename = v.second.data();
                            }
                        
                        }
                    
                        crs.push_back(cr);
                    
                    }
                
                    elly::utils::log() << "  | Parsed " << crs.size() << " factors: " << std::endl;
                    for(int ff = 0; ff < crs.size(); ff++){
                
                        elly::utils::log() << "    + Factor [" << crs[ff].factor_name << "]:" << std::endl;

                        elly::utils::log() << "      - function_id = " << crs[ff].function_id << "" << std::endl;

                        elly::utils::log() << "      - filename = " << crs[ff].filename << "" << std::endl;
                    
                        elly::utils::log() << "      - filetype = " << crs[ff].filetype << "" << std::endl;
                
                    }
                
                    elly::utils::log() << ">> Preparing " << crs.size() << " factors: " << std::endl;
                    for(int ff = 0; ff < crs.size(); ff++){
                    
                        elly::utils::log() << "  | Factor [" << crs[ff].factor_name << "]:" << std::endl;
                    
                        crs[ff].prepare();
                    
                    }
                    
                }
                
            }
            
            
        };
    
    }
}



#endif
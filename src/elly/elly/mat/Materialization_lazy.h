//
//  Materialization_lazy.h
//  elly
//
//  Created by Ce Zhang on 7/28/12.
//  Copyright (c) 2012 University of Wisconsin-Madison. All rights reserved.
//

#ifndef elly_Materialization_lazy_h
#define elly_Materialization_lazy_h

#include "../utils/Common.h"
#include "../utils/FactorFileParser.h"

#include "Materialization.h"

namespace mia{
    namespace elly{
        
        /**
         * \brief Namespace for materialization strategies.
         */
        namespace mat{
                       
            /**
             * \brief Class for lazy materialization. See TR for details.
             **/
            template<hazy::sman::StorageType STORAGE>
            class Materialization_lazy : public mia::elly::mat::Materialization {
                
            public:
                
                /**
                 * mia::elly::utils::FactorFileParser object which provides
                 *   - correlation relations (mia::elly::dstruct::StandardCorrelationRelation or mia::elly::dstruct::IncrementalCorrelationRelation);
                 *   - variable->factor relation (mia::elly::dstruct::VariableFactorRelation)
                 *   - variable->assignment relation (mia::elly::dstruct::VariableAssignmentRelation)
                 *   - variable->tally relation for marginal inference (mia::elly::dstruct::VariableTallyRelation)
                 **/
                mia::elly::utils::FactorFileParser<STORAGE> * parserrs;
              
              void * get_parserrs(){
                return parserrs;
              }
                
                /**
                 * Constructor.
                 *
                 * \param mia::elly::utils::FactorFileParser pointer to mia::elly::utils::FactorFileParser object.
                 *
                 * \sa mia::elly::mat::Materialization_lazy::parserrs
                 *
                 *
                 */
                Materialization_lazy(mia::elly::utils::FactorFileParser<STORAGE> * _parserrs){
                    
                    parserrs = _parserrs;
                    
                }
                                
                /**
                 * Materialze -- for lazy materialization, it is an empty function.
                 *
                 */
                void materialize(){
                    
                    mia::elly::utils::log() << ">> Materializing for Lazy...." << std::endl;
                    
                }
                
                /**
                 * Get number of variables to be sampled. This function is implemented by sending queries to variable-assignment relation. 
                 *
                 * \return int number of variables to be sampled
                 */
                int getNVariable(){
                    return parserrs->va.nvariable;
                }
                
                mia::sm::IntsBlock vt_lookup(int64_t vid){
                  return parserrs->vt.lookup(vid);
                }
              
                int va_lookup(int64_t vid){
                  return parserrs->va.lookup(vid);
                }
              
                int getNCRS(){
                  return parserrs->crs.size();
                }
              
                mia::elly::dstruct::AbstractCorrelationRelation * getCRS(int ncrs){
                  return parserrs->crs[ncrs];
                }
              
                /**
                 * Given a sampled result, update the content all relations.
                 *
                 * \param sampleInput reference to mia::elly::SampleInput object encoding a sample task of one variable
                 * \param newvalue new assignment of the target variable
                 * \param tally true if running marginal inference, which needs tally the value
                 * \param lock true if using lock
                 *
                 */
                void update(mia::elly::SampleInput & sampleInput, int newvalue, bool tally = false, bool lock = true){
                 
                    parserrs->va.set(sampleInput.vid, newvalue);
                    
                    if(tally == true){
                        parserrs->vt.tally(sampleInput.vid, newvalue);
                    }
                    
                    int crid, fid, aux, vpos, funcid;
                    void * mb;
                    
                    for(int nf=0; nf<sampleInput.fids.size();nf ++){
                        
                        crid = sampleInput.crids[nf];
                        fid = sampleInput.fids[nf];
                        aux = sampleInput.auxs[nf];
                        mb = sampleInput.mbs[nf];
                        vpos = sampleInput.pos_of_sample_variable[nf];
                        funcid = sampleInput.funcids[nf];
                        
                        if(vpos == -1){
                            
                            //if(lock == false){
                            //    parserrs->crs[crid]->lock(fid);
                            //}
                            
                            parserrs->crs[crid]->update(fid, funcs_update[funcid], sampleInput.vid, sampleInput.vvalue, newvalue, !lock);
                            
                            //if(lock == false){
                            //    parserrs->crs[crid]->release(fid);
                            //}
                            
                        }
                        
                    }
                
                    if(lock == true){
                        
                        for(int nf=0; nf<sampleInput.fids.size();nf ++){
                    
                            crid = sampleInput.crids[nf];
                            fid = sampleInput.fids[nf];
                        
                        //todo: should we lock variables?
                            parserrs->crs[crid]->release(fid);
                    
                        }
                    }
                    
                    
                }

                
                // get mia::elly:SampleInput object as input to sampler
                void retrieve(int vid, mia::elly::SampleInput & rs, bool train = false, bool lock = true){
                    
                    for(int i=0;i<rs.mbs.size();i++){
                        delete rs.mbs[i];
                    }
                    
                    rs.aux2s.clear();
                    rs.auxs.clear();
                    rs.pos_of_sample_variable.clear();
                    rs.weights.clear();
                    rs.mbs.clear();
                    rs.crids.clear();
                    rs.fids.clear();
                    rs.funcids.clear();
                    
                    if(train == true){
                        rs.vtrain = parserrs->vtrain.lookup(vid);
                    }else{
                        rs.vtrain = -2;
                    }
                    
                    // first get a list of factors of vid
                    mia::sm::IntsBlock factors = parserrs->vf.lookup(vid);
                    
                    int crid;
                    int fid;
                    int mbvid;
                    int mbvvalue;
                    int aux;
                    int aux2;
                    
                    int currentNFactor = 0;
                    int currentNVariable = 0;
                    
                    
                    if(lock == true){
                        for(int factor=1;factor<factors.size;factor+=2){
                            crid = factors.get<int>(factor);
                            fid = factors.get<int>(factor+1);
                            //std::cout << "lock " << fid << std::endl;
                            
                            parserrs->crs[crid]->lock(fid); // add warning for dead lock --- add a checker
                        }
                    }
                    
                    //std::cout << "###" << std::endl;
                    
                    rs.vid = vid;
                    rs.vvalue = parserrs->va.lookup(vid);
                    rs.vdomain = parserrs->va.lookup_domain(vid);
                    
                    // for each factor
                    for(int factor=1;factor<factors.size;factor+=2){
                        
                        crid = factors.get<int>(factor);
                        fid = factors.get<int>(factor+1);  
                        
                        rs.crids.push_back(crid);
                        rs.fids.push_back(fid);
                        
                        rs.funcids.push_back(parserrs->crs[crid]->function_id);
                        rs.weights.push_back(&parserrs->crs[crid]->weights);
                                                
                        // if not incremental factor
                        if(funcs_incremental[parserrs->crs[crid]->function_id] == false){
                        
                            std::vector<int> * empty = new std::vector<int>;
                            
                            //std::cout << crid << ", " << fid << std::endl;
                            mia::sm::IntsBlock * mb =
                                (mia::sm::IntsBlock*) parserrs->crs[crid]->lookup(fid);
                        
                            aux = mb->get<int>(0);
                            
                            //std::cout << aux << std::endl;
                            
                            aux2 = mb->get<int>(1);
                            
                            //std::cout << aux2 << " @ " << mb->size << std::endl;
                            
                            rs.auxs.push_back(aux);
                            rs.aux2s.push_back(aux2);
                            
                            // for each variables in that factor, fetch current values
                            currentNVariable = 0;
                        
                            //std::cout << "mb->size = " << mb->size << std::endl;
                        
                            for(int nmbv=2; nmbv < mb->size; nmbv ++){
                            
                                mbvid = mb->get<int>(nmbv);
                                mbvvalue = parserrs->va.lookup(mbvid);
                            
                                //std::cout << mbvid << " = " << mbvvalue << "mb->size = " << mb->size << std::endl;
                            
                                if(mbvid == vid){
                          
                                    //std::cout << rs.pos_of_sample_variable.size() << "~~~" << currentNFactor << std::endl;
                                
                                    assert(rs.pos_of_sample_variable.size() == currentNFactor); // variable ID is distinct in each factor! same varialbe do not appear twice.
                                
                                    rs.pos_of_sample_variable.push_back(currentNVariable);
                                
                                }
                            
                                //std::cout << "pushed_value = " << mbvvalue << std::endl;
                                empty->push_back(mbvvalue);
                            
                                currentNVariable ++;
                            
                            }
                                                
                            delete (mia::sm::IntsBlock *) mb;
                            rs.mbs.push_back((void*)empty);
                            
                        }else{
                            
                            rs.auxs.push_back(-1);
                            rs.aux2s.push_back(-1);
                            
                            rs.pos_of_sample_variable.push_back(-1);
                            void * mb = (void*) parserrs->crs[crid]->lookup(fid);
                            
                            //std::cout << mb << std::endl;
                            
                            rs.mbs.push_back(mb);
                            
                        }
                        
                        currentNFactor ++;
                    }
                    
                    //rs.print();
                    
                }
                                
            };
        
        }
    }
}



#endif

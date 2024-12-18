#include <vector>
#include <string.h>
#include <cmath>
#include <fstream>
#include <limits>
#include <limits>
#include <iomanip>
#include <gtest/gtest.h>
#include <iostream>
#include <cstdio>

#include <cstdlib>
#include "archlab.h"
#include"fast_URBG.hpp"
#include <unistd.h>
#include <algorithm>
#include <cstdint>
#include"function_map.hpp"
#include <vector>
#include <dlfcn.h>
#include "perfstats.h"

#define ELEMENT_TYPE uint64_t

uint array_size; 

typedef void(*seven_seg_impl)(char **, int32_t*, uint32_t);

int main(int argc, char *argv[])
{

	

	    int i, reps=1, size, iterations=1,mhz, verify =0;
        char *stat_file = NULL;
        char default_filename[] = "stats.csv";
        char preamble[1024];
        char epilogue[1024];
        char header[1024];
    	std::vector<int> mhz_s;
	    std::vector<int> default_mhz;
	    std::stringstream clocks;
	    std::vector<std::string> functions;
	    std::vector<std::string> default_functions;
	    std::vector<unsigned long int> sizes;
	    std::vector<unsigned long int> default_sizes;
	    default_sizes.push_back(16);
	
	    std::vector<uint64_t> seeds;
	    std::vector<uint64_t> default_seeds;
	    default_seeds.push_back(0xDEADBEEF);
        for(i = 1; i < argc; i++)
        {
            // This is an option.
            if(argv[i][0]=='-')
            {
                switch(argv[i][1])
                {
                    case 'o':
		            if(i+1 < argc && argv[i+1][0]!='-')
                            stat_file = argv[i+1];
                        break;
                    case 'r':
		            if(i+1 < argc && argv[i+1][0]!='-')
                            reps = atoi(argv[i+1]);
                        break;
		            case 's':
		            for(;i+1<argc;i++)
		            {
		                if(argv[i+1][0]!='-')
		                {
		                    size = atoi(argv[i+1]);
	                        sizes.push_back(size);
                        }
			            else
			                break;
		            }
		            break;

		            case 'M':
                    for(;i+1<argc;i++)
                    {
                        if(argv[i+1][0]!='-')
                        {
                            mhz = atoi(argv[i+1]);
                                mhz_s.push_back(mhz);
                    }
                    else
                        break;
                    }
                    break;
                    case 'f':
                        for(;i+1<argc;i++)
                        {
                            if(argv[i+1][0]!='-')
                            {
                                        functions.push_back(std::string(argv[i+1]));
                            }
                        else
                            break;
                        }
                        break;
                    case 'i':
                        if(i+1 < argc && argv[i+1][0]!='-')
                                    iterations = atoi(argv[i+1]);
                        break;
                    case 'h':
                        break;
                    case 'v':
                           verify = 1;
                        break;

                }
            }
        }
	if(stat_file==NULL)
	    stat_file = default_filename;

	if (std::find(functions.begin(), functions.end(), "ALL") != functions.end()) {
		functions.clear();
		for(auto & f : function_map::get()) {
			functions.push_back(f.first);
		}
	}
	
	for(auto & function : functions) {
		auto t= function_map::get().find(function);
		if (t == function_map::get().end()) {
			std::cerr << "Unknown function: " << function <<"\n";
			exit(1);
		}
		std::cerr << "Gonna run " << function << "\n";
	}
	if(sizes.size()==0)
	    sizes = default_sizes;
	if(seeds.size()==0)
	    seeds = default_seeds;
	if(functions.size()==0)
	    functions = default_functions;
	if(verify == 1)
	   sprintf(header,"size,iterations,function,IC,Cycles,CPI,CT,ET,L1_dcache_miss_rate,L1_dcache_misses,L1_dcache_accesses,branches,branch_misses,correctness");
	else
	   sprintf(header,"size,iterations,function,IC,Cycles,CPI,CT,ET,L1_dcache_miss_rate,L1_dcache_misses,L1_dcache_accesses,branches,branch_misses");
        perfstats_print_header(stat_file, header);
     
	for(auto & seed: seeds ) {
		for(auto & size:sizes) {
//			for(auto & density: densities ) {
				int *source = new int[size];
				for (unsigned int i = 0; i < size; i++) {
					source[i] = (int)(fast_rand(&seed)&0xFFFFFFFF);
				}
				char **result = new char*[size];
				char *temp = new char[12*size];
				
				for(int i = 0; i < size; i++, temp+=12)
				    result[i] = temp;
				for(auto & function : functions) {
					std::cerr << "Running: " << function << "\n";
                    
                    memset(result[0], 0, 12*size);
					function_spec_t f_spec = function_map::get()[function];
					auto fut = reinterpret_cast<seven_seg_impl>(f_spec.second);
					sprintf(preamble, "%lu,%d,%s,",size,iterations,function.c_str());
                    
					perfstats_init();
					perfstats_enable(1);
                    for(int j=0; j < iterations; j++ ) {
					fut(result, source, size);
                    }
					perfstats_disable(1);
					if(verify)
					{
						if(function.find("seven_seg_solution_c") != std::string::npos)
						{
                            int correctness = 1;
							char **verify = new char*[size];
							char *temp_verify = new char[12*size];
							memset(temp_verify, 0, 12*size);
							for(int i = 0; i < size; i++, temp_verify+=12)
								verify[i] = temp_verify;
							function_spec_t t = function_map::get()[std::string("seven_seg_reference_c")];
							auto verify_fut = reinterpret_cast<seven_seg_impl>(t.second);
							verify_fut(verify, source, size);
							for(int i = 0 ; i < size; i++)
							{
							    for(int j =0; j < 12 ; j++)
							    if(verify[i][j] != result[i][j])
							    {
							        std::cerr << "Incorrect result!" << i << ","<< (int)result[i][j] <<"," << (int)verify[i][j]<<std::endl;
                                    correctness = 0;
							    	break;
							    }
							}
                            sprintf(epilogue,",%d\n",correctness);
						}
						else
						    sprintf(epilogue,",0\n");
					}
					else
						sprintf(epilogue,"\n");
					perfstats_print(preamble, stat_file, epilogue);
					perfstats_deinit();
					std::cerr << "Done execution: " << function << "\n";
				}
//			}
		}
	}
	return 0;
}

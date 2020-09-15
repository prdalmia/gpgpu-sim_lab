// Copyright (c) 2009-2011, Tor M. Aamodt
// The University of British Columbia
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
// Neither the name of The University of British Columbia nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <list>
#include <set>

#include "../option_parser.h"
#include "mem_fetch.h"
#include "dram.h"
#include "gpu-cache.h"
#include "histogram.h"
#include "l2cache.h"
#include "../statwrapper.h"
#include "../abstract_hardware_model.h"
#include "gpu-sim.h"
#include "shader.h"
#include "mem_latency_stat.h"
#include "l2cache_trace.h"


mem_fetch * partition_mf_allocator::alloc(new_addr_type addr, mem_access_type type, unsigned size, bool wr ) const 
{
    assert( wr );
    mem_access_t access( type, addr, size, wr );
    mem_fetch *mf = new mem_fetch( access, 
                                   NULL,
                                   WRITE_PACKET_SIZE, 
                                   -1, 
                                   -1, 
                                   -1,
                                   m_memory_config );
    return mf;
}

memory_partition_unit::memory_partition_unit( unsigned partition_id, 
                                              const struct memory_config *config,
                                              class memory_stats_t *stats )
: m_id(partition_id), m_config(config), m_stats(stats), m_arbitration_metadata(config) 
{
    m_dram = new dram_t(m_id,m_config,m_stats,this);

    m_sub_partition = new memory_sub_partition*[m_config->m_n_sub_partition_per_memory_channel]; 
    for (unsigned p = 0; p < m_config->m_n_sub_partition_per_memory_channel; p++) {
        unsigned sub_partition_id = m_id * m_config->m_n_sub_partition_per_memory_channel + p; 
        m_sub_partition[p] = new memory_sub_partition(sub_partition_id, m_config, stats); 
    }
}

void memory_partition_unit::handle_memcpy_to_gpu( size_t addr, unsigned global_subpart_id, mem_access_sector_mask_t mask )
{
    unsigned p = global_sub_partition_id_to_local_id(global_subpart_id);
    std::string mystring =
        mask.to_string<char,std::string::traits_type,std::string::allocator_type>();
    MEMPART_DPRINTF("Copy Engine Request Received For Address=%llx, local_subpart=%u, global_subpart=%u, sector_mask=%s \n", addr, p, global_subpart_id, mystring.c_str()); 
    m_sub_partition[p]->force_l2_tag_update(addr,gpu_sim_cycle+gpu_tot_sim_cycle, mask);
}

memory_partition_unit::~memory_partition_unit() 
{
    delete m_dram; 
    for (unsigned p = 0; p < m_config->m_n_sub_partition_per_memory_channel; p++) {
        delete m_sub_partition[p]; 
    } 
    delete[] m_sub_partition; 
}

memory_partition_unit::arbitration_metadata::arbitration_metadata(const struct memory_config *config) 
: m_last_borrower(config->m_n_sub_partition_per_memory_channel - 1), 
  m_private_credit(config->m_n_sub_partition_per_memory_channel, 0), 
  m_shared_credit(0) 
{
    // each sub partition get at least 1 credit for forward progress 
    // the rest is shared among with other partitions 
    m_private_credit_limit = 1; 
    m_shared_credit_limit = config->gpgpu_frfcfs_dram_sched_queue_size 
                            + config->gpgpu_dram_return_queue_size 
                            - (config->m_n_sub_partition_per_memory_channel - 1);
    if(config->seperate_write_queue_enabled )
    	m_shared_credit_limit += config->gpgpu_frfcfs_dram_write_queue_size;
    if (config->gpgpu_frfcfs_dram_sched_queue_size == 0 
        or config->gpgpu_dram_return_queue_size == 0) 
    {
        m_shared_credit_limit = 0; // no limit if either of the queue has no limit in size 
    }
    assert(m_shared_credit_limit >= 0); 
}

bool memory_partition_unit::arbitration_metadata::has_credits(int inner_sub_partition_id) const 
{
    int spid = inner_sub_partition_id; 
    if (m_private_credit[spid] < m_private_credit_limit) {
        return true; 
    } else if (m_shared_credit_limit == 0 || m_shared_credit < m_shared_credit_limit) {
        return true; 
    } else {
        return false; 
    }
}

void memory_partition_unit::arbitration_metadata::borrow_credit(int inner_sub_partition_id) 
{
    int spid = inner_sub_partition_id; 
    if (m_private_credit[spid] < m_private_credit_limit) {
        m_private_credit[spid] += 1; 
    } else if (m_shared_credit_limit == 0 || m_shared_credit < m_shared_credit_limit) {
        m_shared_credit += 1; 
    } else {
        assert(0 && "DRAM arbitration error: Borrowing from depleted credit!"); 
    }
    m_last_borrower = spid; 
}

void memory_partition_unit::arbitration_metadata::return_credit(int inner_sub_partition_id) 
{
    int spid = inner_sub_partition_id; 
    if (m_private_credit[spid] > 0) {
        m_private_credit[spid] -= 1; 
    } else {
        m_shared_credit -= 1; 
    } 
    assert((m_shared_credit >= 0) && "DRAM arbitration error: Returning more than available credits!"); 
}

void memory_partition_unit::arbitration_metadata::print( FILE *fp ) const 
{
    fprintf(fp, "private_credit = "); 
    for (unsigned p = 0; p < m_private_credit.size(); p++) {
        fprintf(fp, "%d ", m_private_credit[p]); 
    }
    fprintf(fp, "(limit = %d)\n", m_private_credit_limit); 
    fprintf(fp, "shared_credit = %d (limit = %d)\n", m_shared_credit, m_shared_credit_limit); 
}

bool memory_partition_unit::busy() const 
{
    bool busy = false; 
    for (unsigned p = 0; p < m_config->m_n_sub_partition_per_memory_channel; p++) {
        if (m_sub_partition[p]->busy()) {
            busy = true; 
        }
    }
    return busy; 
}

void memory_partition_unit::cache_cycle(unsigned cycle) 
{
    for (unsigned p = 0; p < m_config->m_n_sub_partition_per_memory_channel; p++) {
        m_sub_partition[p]->cache_cycle(cycle); 
    }
}

void memory_partition_unit::visualizer_print( gzFile visualizer_file ) const 
{
    m_dram->visualizer_print(visualizer_file);
    for (unsigned p = 0; p < m_config->m_n_sub_partition_per_memory_channel; p++) {
        m_sub_partition[p]->visualizer_print(visualizer_file); 
    }
}

// determine whether a given subpartition can issue to DRAM 
bool memory_partition_unit::can_issue_to_dram(int inner_sub_partition_id) 
{
    int spid = inner_sub_partition_id; 
    bool sub_partition_contention = m_sub_partition[spid]->dram_L2_queue_full(); 
    bool has_dram_resource = m_arbitration_metadata.has_credits(spid); 

    MEMPART_DPRINTF("sub partition %d sub_partition_contention=%c has_dram_resource=%c\n", 
                    spid, (sub_partition_contention)? 'T':'F', (has_dram_resource)? 'T':'F'); 

    return (has_dram_resource && !sub_partition_contention); 
}

int memory_partition_unit::global_sub_partition_id_to_local_id(int global_sub_partition_id) const
{
    return (global_sub_partition_id - m_id * m_config->m_n_sub_partition_per_memory_channel); 
}

void memory_partition_unit::dram_cycle() 
{ 
    // pop completed memory request from dram and push it to dram-to-L2 queue 
    // of the original sub partition 
    mem_fetch* mf_return = m_dram->return_queue_top();
    if (mf_return) {
        unsigned dest_global_spid = mf_return->get_sub_partition_id(); 
        int dest_spid = global_sub_partition_id_to_local_id(dest_global_spid); 
        assert(m_sub_partition[dest_spid]->get_id() == dest_global_spid); 
        if (!m_sub_partition[dest_spid]->dram_L2_queue_full()) {
            if( mf_return->get_access_type() == L1_WRBK_ACC ) {
                m_sub_partition[dest_spid]->set_done(mf_return); 
                delete mf_return;
            } else {
                m_sub_partition[dest_spid]->dram_L2_queue_push(mf_return);
                mf_return->set_status(IN_PARTITION_DRAM_TO_L2_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
                m_arbitration_metadata.return_credit(dest_spid); 
                MEMPART_DPRINTF("mem_fetch request %p return from dram to sub partition %d\n", mf_return, dest_spid); 
            }
            m_dram->return_queue_pop(); 
        }
    } else {
        m_dram->return_queue_pop(); 
    }
    
    m_dram->cycle(); 
    m_dram->dram_log(SAMPLELOG);   

   // mem_fetch *mf = m_sub_partition[spid]->L2_dram_queue_top();
    //if( !m_dram->full(mf->is_write()) ) {
        // L2->DRAM queue to DRAM latency queue
        // Arbitrate among multiple L2 subpartitions 
        int last_issued_partition = m_arbitration_metadata.last_borrower(); 
        for (unsigned p = 0; p < m_config->m_n_sub_partition_per_memory_channel; p++) {
            int spid = (p + last_issued_partition + 1) % m_config->m_n_sub_partition_per_memory_channel; 
            if (!m_sub_partition[spid]->L2_dram_queue_empty() && can_issue_to_dram(spid)) {
                mem_fetch *mf = m_sub_partition[spid]->L2_dram_queue_top();
                if(m_dram->full(mf->is_write()) )
                	break;

                m_sub_partition[spid]->L2_dram_queue_pop();
                MEMPART_DPRINTF("Issue mem_fetch request %p from sub partition %d to dram\n", mf, spid); 
                dram_delay_t d;
                d.req = mf;
                d.ready_cycle = gpu_sim_cycle+gpu_tot_sim_cycle + m_config->dram_latency;
                m_dram_latency_queue.push_back(d);
                mf->set_status(IN_PARTITION_DRAM_LATENCY_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
                m_arbitration_metadata.borrow_credit(spid); 
                break;  // the DRAM should only accept one request per cycle 
            }
        }
    //}

    // DRAM latency queue

    if( !m_dram_latency_queue.empty() && ( (gpu_sim_cycle+gpu_tot_sim_cycle) >= m_dram_latency_queue.front().ready_cycle ) && !m_dram->full(m_dram_latency_queue.front().req->is_write()) ) {
    	mem_fetch* mf = m_dram_latency_queue.front().req;
    	m_dram_latency_queue.pop_front();
        m_dram->push(mf);
    }
}

void memory_partition_unit::set_done( mem_fetch *mf )
{
    unsigned global_spid = mf->get_sub_partition_id(); 
    int spid = global_sub_partition_id_to_local_id(global_spid); 
    assert(m_sub_partition[spid]->get_id() == global_spid); 
    if (mf->get_access_type() == L1_WRBK_ACC || mf->get_access_type() == L2_WRBK_ACC) {
        m_arbitration_metadata.return_credit(spid); 
        MEMPART_DPRINTF("mem_fetch request %p return from dram to sub partition %d\n", mf, spid); 
    }
    m_sub_partition[spid]->set_done(mf); 
}

void memory_partition_unit::set_dram_power_stats(unsigned &n_cmd,
                                                 unsigned &n_activity,
                                                 unsigned &n_nop,
                                                 unsigned &n_act,
                                                 unsigned &n_pre,
                                                 unsigned &n_rd,
                                                 unsigned &n_wr,
                                                 unsigned &n_req) const
{
    m_dram->set_dram_power_stats(n_cmd, n_activity, n_nop, n_act, n_pre, n_rd, n_wr, n_req);
}

void memory_partition_unit::print( FILE *fp ) const
{
    fprintf(fp, "Memory Partition %u: \n", m_id); 
    for (unsigned p = 0; p < m_config->m_n_sub_partition_per_memory_channel; p++) {
        m_sub_partition[p]->print(fp); 
    }
    fprintf(fp, "In Dram Latency Queue (total = %zd): \n", m_dram_latency_queue.size()); 
    for (std::list<dram_delay_t>::const_iterator mf_dlq = m_dram_latency_queue.begin(); 
         mf_dlq != m_dram_latency_queue.end(); ++mf_dlq) {
        mem_fetch *mf = mf_dlq->req; 
        fprintf(fp, "Ready @ %llu - ", mf_dlq->ready_cycle); 
        if (mf) 
            mf->print(fp); 
        else 
            fprintf(fp, " <NULL mem_fetch?>\n"); 
    }
    m_dram->print(fp); 
}

memory_sub_partition::memory_sub_partition( unsigned sub_partition_id, 
                                            const struct memory_config *config,
                                            class memory_stats_t *stats )
{
    m_id = sub_partition_id;
    m_config=config;
    m_stats=stats;
    m_memcpy_cycle_offset = 0;

    assert(m_id < m_config->m_n_mem_sub_partition); 

    char L2c_name[32];
    snprintf(L2c_name, 32, "L2_bank_%03d", m_id);
    m_L2interface = new L2interface(this);
    m_mf_allocator = new partition_mf_allocator(config);

    if(!m_config->m_L2_config.disabled())
       m_L2cache = new l2_cache(L2c_name,m_config->m_L2_config,-1,-1,m_L2interface,m_mf_allocator,IN_PARTITION_L2_MISS_QUEUE);

    unsigned int icnt_L2;
    unsigned int L2_dram;
    unsigned int dram_L2;
    unsigned int L2_icnt;
    sscanf(m_config->gpgpu_L2_queue_config,"%u:%u:%u:%u", &icnt_L2,&L2_dram,&dram_L2,&L2_icnt );
    m_icnt_L2_queue = new fifo_pipeline<mem_fetch>("icnt-to-L2",0,icnt_L2); 
    m_L2_dram_queue = new fifo_pipeline<mem_fetch>("L2-to-dram",0,L2_dram);
    m_dram_L2_queue = new fifo_pipeline<mem_fetch>("dram-to-L2",0,dram_L2);
    m_L2_icnt_queue = new fifo_pipeline<mem_fetch>("L2-to-icnt",0,L2_icnt);
    wb_addr=-1;
}

memory_sub_partition::~memory_sub_partition()
{
    delete m_icnt_L2_queue;
    delete m_L2_dram_queue;
    delete m_dram_L2_queue;
    delete m_L2_icnt_queue;
    delete m_L2cache;
    delete m_L2interface;
}

void memory_sub_partition:: cache_cycle( unsigned cycle )
{
    // L2 fill responses
    if( !m_config->m_L2_config.disabled()) {
       if ( m_L2cache->access_ready() && !m_L2_icnt_queue->full() ) {
           mem_fetch *mf = m_L2cache->next_access();
           if(mf->get_access_type() != L2_WR_ALLOC_R){ // Don't pass write allocate read request back to upper level cache
				mf->set_reply();
				mf->set_status(IN_PARTITION_L2_TO_ICNT_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
				m_L2_icnt_queue->push(mf);
           }else{
        	    if(m_config->m_L2_config.m_write_alloc_policy == FETCH_ON_WRITE)
        	    {
        	    	mem_fetch* original_wr_mf = mf->get_original_wr_mf();
					assert(original_wr_mf);
					original_wr_mf->set_reply();
					original_wr_mf->set_status(IN_PARTITION_L2_TO_ICNT_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
					m_L2_icnt_queue->push(original_wr_mf);
        	    }
				m_request_tracker.erase(mf);
				delete mf;
           }
       }
    }

    // DRAM to L2 (texture) and icnt (not texture)
    if ( !m_dram_L2_queue->empty() ) {
        mem_fetch *mf = m_dram_L2_queue->top();
        if ( !m_config->m_L2_config.disabled() && m_L2cache->waiting_for_fill(mf) ) {
            if (m_L2cache->fill_port_free()) {
                mf->set_status(IN_PARTITION_L2_FILL_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
                m_L2cache->fill(mf,gpu_sim_cycle+gpu_tot_sim_cycle+m_memcpy_cycle_offset);
                m_dram_L2_queue->pop();
            }
        } else if ( !m_L2_icnt_queue->full() ) {
        	if(mf->is_write() && mf->get_type() == WRITE_ACK)
            mf->set_status(IN_PARTITION_L2_TO_ICNT_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
            m_L2_icnt_queue->push(mf);
            m_dram_L2_queue->pop();
        }
    }

    // prior L2 misses inserted into m_L2_dram_queue here
    if( !m_config->m_L2_config.disabled() )
       m_L2cache->cycle();

    // new L2 texture accesses and/or non-texture accesses
    if ( !m_L2_dram_queue->full() && !m_icnt_L2_queue->empty() ) {
        mem_fetch *mf = m_icnt_L2_queue->top();
        if ( !m_config->m_L2_config.disabled() &&
              ( (m_config->m_L2_texure_only && mf->istexture()) || (!m_config->m_L2_texure_only) )
           ) {
            // L2 is enabled and access is for L2
            bool output_full = m_L2_icnt_queue->full(); 
            bool port_free = m_L2cache->data_port_free(); 
            if ( !output_full && port_free ) {
                std::list<cache_event> events;
                //stop replacement till there are pending requests to the same address
                enum cache_request_status status = m_L2cache->access(mf->get_addr(),mf,gpu_sim_cycle+gpu_tot_sim_cycle+m_memcpy_cycle_offset,events);
                if(mf->get_type() == EVICTION){
                    if (status == REMOTE_OWNED){
                    printf("EVICTION headed for a remote owned evicted line is %d\n", status);
                    }
                }
                     
                //CAN WE GET A SECTOR MISS ?
                bool write_sent = was_write_sent(events);
                bool read_sent = was_read_sent(events);
                if(status == MISS || status == HIT){   
                    unsigned int cache_index;
                    m_L2cache->process_probe(mf ,cache_index); 
                     unsigned cache_pending_index = m_L2cache->get_ownership_pending_index(mf, get_id());
                      if (cache_pending_index != unsigned(-1)){
                    //      printf("Changing cache_index for address %x from %d to %d and is atomic %d where ID is %d\n", mf->get_addr(), cache_index, cache_pending_index, mf->isatomic(), get_id());
                          cache_index = cache_pending_index;
                          status = REMOTE_OWNED;
                      }
                if(mf->isatomic() && (m_L2cache->get_owner(mf, cache_index) == (unsigned)-1) && status != REMOTE_OWNED){
                                 m_L2cache->set_owner( mf, cache_index, mf->get_sid());
                          
                                 if((mf->get_addr() & (new_addr_type)(~127)) == 0xc08ccb00){
                         printf("Owner is  core %d for address %x going to cache_index %d and memory partition %d\n", mf->get_sid() ,mf->get_addr(), cache_index, get_id());
                         }
                              
                                 m_L2cache->add_ownership_champion(mf, cache_index, get_id());
                                 m_L2cache->add_ownership_pending_index(mf, cache_index, get_id());
                                 
                            }
                MEM_SUBPART_DPRINTF("Probing L2 cache Address=%llx, status=%u\n", mf->get_addr(), status); 
                }
                if ( status == HIT ) {
                    if( !write_sent ) {
                        // L2 cache replies
                        assert(!read_sent);
                        if( mf->get_access_type() == L1_WRBK_ACC ) {
                            m_request_tracker.erase(mf);
                            delete mf;
                        } else {
                            mf->set_reply();
                            mf->set_status(IN_PARTITION_L2_TO_ICNT_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
                            m_L2_icnt_queue->push(mf);
                        }
                        m_icnt_L2_queue->pop();
                    } else {
                        assert(write_sent);
                        m_icnt_L2_queue->pop();
                    }
                }
                
                else if (status == REMOTE_OWNED)  {
                    unsigned int cache_index;
                  enum cache_request_status probe_status =  m_L2cache->process_probe(mf , cache_index);
                 if(((mf->get_sid() == m_L2cache->get_owner (mf, cache_index)) || ((m_L2cache->get_owner (mf, cache_index) == unsigned(-1)) && mf->get_type() == INVALIDATION_RESPONSE)) && (m_L2cache->get_line_address(mf, cache_index) == (mf->get_addr()  & ~(new_addr_type)(127)))){ //need to change this logic
                          //L2 cache will check if somebody is waiting for ownership at that address
                         // if yes make that next request the current owner and send the block to the new owner
                     // This needs to be replaced by index?
                     
                      mem_fetch * mf_pending = m_L2cache->get_waiting_for_ownership(mf, cache_index);
                     
                     if (mf_pending){
                      mf_pending->set_reply();
                      mf_pending->set_status(IN_PARTITION_L2_TO_ICNT_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
                      m_L2cache->remove_ownership_pending_index(mf_pending, get_id()); 
                  
                       if(((mf->get_addr() & (new_addr_type)(~127)) == 0xc08ccb00)){
                       printf("Invalidation response recieved from core %d for address %x going to cache_index %d and memory partition %d \n", mf->get_sid(), mf->get_addr(), cache_index, get_id());
                       }
                    
                     if(m_L2cache->get_line_address(mf, cache_index) != (mf_pending->get_addr() & ~(new_addr_type)(127))){
                      m_L2cache->allocate(mf_pending, cache_index, gpu_sim_cycle+gpu_tot_sim_cycle);
                      }
                      
                      if(mf_pending->isatomic() == true){
                      m_L2cache->set_owner( mf_pending, cache_index, mf_pending->get_sid()); //CHANGE TO LINE ADDRESS
                      }
                      else{
                          /*
                          if(mf_pending->get_type() == EVICTION){
                              printf("Uh oh we have a problem here for address %d\n", mf_pending->get_addr());
                          }
                          */
                      //    printf("What a i doing here %d from core %d for address %x\n", mf_pending->get_type(), mf_pending->get_sid(), mf_pending->get_addr());
                          m_L2cache->set_owner( mf_pending, cache_index, unsigned(-1));
                          // Here I can choose to completely empty out waiting for ownership and ownership champion queues
                          mem_fetch* temp = m_L2cache->get_waiting_for_ownership(mf, cache_index);
                          if(temp == NULL){  
                          m_L2cache->remove_from_ownership_champion_queue(cache_index, get_id(), 2);
                          }
                      }

                      m_L2cache->remove_from_ownership_queue(cache_index);
                      m_L2_icnt_queue->push(mf_pending);
                  
                      //pop request from waiting for ownership queue
                     }
                      else{
                          assert(m_L2cache->get_ownership_champion(mf, cache_index) == mf->get_sid());
                          m_L2cache->remove_from_ownership_champion_queue(cache_index, get_id(), 3);
                          m_L2cache->set_owner( mf, cache_index, (unsigned)-1); //CHANGE TO LINE ADDRESS
                  // while( !ownership_champion[(mf->get_addr() & ~(new_addr_type)(m_config->m_L2_config.c_sz-1))].empty())
                  // {
                  //     ownership_champion[(mf->get_addr() & ~(new_addr_type)(m_config->m_L2_config.m_line_sz-1))].pop();
                  // }

                      }
                      m_icnt_L2_queue->pop();
                       if(mf->get_type() == INVALIDATION_RESPONSE){
                       m_request_tracker.erase(mf);
                   }
                      }
                     else{
                            if (mf->get_type() == INVALIDATION_RESPONSE)
                             {
                                 printf("The invalidation response for address %x should not be in this section with cache index %d, core %d and memory %d and is atomic %d\n",mf->get_addr(), cache_index, mf->get_sid(), get_id(), mf->isatomic());
                                 throw std::runtime_error("You are at a bad place man");
                             }

                     
                         if(((mf->get_addr() & (new_addr_type)(~127)) == 0xc08ccb00)){
                         printf("Request from core %d for address %x going to cache_index %d and memory partition %d and is atomic %d\n", mf->get_sid() ,mf->get_addr(), cache_index, get_id(), mf->isatomic());
                         }
                    
                           m_L2cache->add_ownership_pending_index(mf, cache_index, get_id());
                            
                       
                         m_L2cache->add_waiting_for_ownership(mf, cache_index);
                         m_L2cache->add_ownership_champion(mf, cache_index, get_id());


                         // assert(!(ownership_champion[(mf->get_addr() & ~(new_addr_type)(m_config->m_L2_config.m_line_sz-1))].empty()));
                         //need to add this to the ownership champion queue
                         unsigned invalidation_reciever = m_L2cache->get_ownership_champion(mf, cache_index);
                         new_addr_type invalidation_reciever_address = m_L2cache->get_ownership_champion_address(cache_index);
                  
                                mem_access_t access( mf->get_access_type(),  invalidation_reciever_address, mf->get_ctrl_size(), false);
                               // you also have to do the stuff which would have happenend if there was a replacement
                                //if((mf->get_addr() & (new_addr_type)(~127)) == 0xc0933480){
                                 //printf("Invalidation Sent to core %d for address %x\n", invalidation_reciever,m_L2cache->get_line_address(mf, cache_index));
                                //}
                                unsigned cluster_id = invalidation_reciever/2;
                                mem_fetch *mf_flush = new mem_fetch( access, 
                                               NULL,
                                               mf->get_ctrl_size(), 
                                               -1, 
                                               invalidation_reciever, 
                                               cluster_id,
                                               mf->get_mem_config() );
                                mf_flush->set_type(INVALIDATION);
                    /*
                            if(invalidation_reciever_address & (new_addr_type)(~127) == 0xc08ccb00){
                         printf("Invalidation sent to core %d for address %x where the incoming address is %x and cache_index is %d and memory partition id is %d\n", invalidation_reciever , invalidation_reciever_address, mf->get_addr(), cache_index, get_id());
                         }                               
                      */  
                          m_L2_icnt_queue->push(mf_flush);
                                mf_flush->set_status(IN_PARTITION_L2_TO_ICNT_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
                                // L2 cache accepted request
                                m_L2cache->remove_from_ownership_champion_queue(cache_index, get_id(), 1);
                          m_icnt_L2_queue->pop();
                       

                     } 

                }
                 else if ( status != RESERVATION_FAIL ) {
                	if(mf->is_write() && (m_config->m_L2_config.m_write_alloc_policy == FETCH_ON_WRITE || m_config->m_L2_config.m_write_alloc_policy == LAZY_FETCH_ON_READ) && !was_writeallocate_sent(events)) {
                		mf->set_reply();
                		mf->set_status(IN_PARTITION_L2_TO_ICNT_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
                		m_L2_icnt_queue->push(mf);
                	}
                    // L2 cache accepted request
                    m_icnt_L2_queue->pop();
                } else {
                    assert(!write_sent);
                    assert(!read_sent);
                    // L2 cache lock-up: will try again next cycle
                }
            }
        } else {
            // L2 is disabled or non-texture access to texture-only L2
            mf->set_status(IN_PARTITION_L2_TO_DRAM_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
            m_L2_dram_queue->push(mf);
            m_icnt_L2_queue->pop();
        }
    }

    // ROP delay queue
    if( !m_rop.empty() && (cycle >= m_rop.front().ready_cycle) && !m_icnt_L2_queue->full() ) {
        mem_fetch* mf = m_rop.front().req;
        m_rop.pop();
        m_icnt_L2_queue->push(mf);
        mf->set_status(IN_PARTITION_ICNT_TO_L2_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
    }
}

bool memory_sub_partition::full() const
{
    return m_icnt_L2_queue->full();
}

bool memory_sub_partition::full(unsigned size) const
{
    return m_icnt_L2_queue->is_avilable_size(size);
}

bool memory_sub_partition::L2_dram_queue_empty() const
{
   return m_L2_dram_queue->empty(); 
}

class mem_fetch* memory_sub_partition::L2_dram_queue_top() const
{
   return m_L2_dram_queue->top(); 
}

void memory_sub_partition::L2_dram_queue_pop() 
{
   m_L2_dram_queue->pop(); 
}

bool memory_sub_partition::dram_L2_queue_full() const
{
   return m_dram_L2_queue->full(); 
}

void memory_sub_partition::dram_L2_queue_push( class mem_fetch* mf )
{
   m_dram_L2_queue->push(mf); 
}

void memory_sub_partition::print_cache_stat(unsigned &accesses, unsigned &misses) const
{
    FILE *fp = stdout;
    if( !m_config->m_L2_config.disabled() )
       m_L2cache->print(fp,accesses,misses);
}

void memory_sub_partition::print( FILE *fp ) const
{
    if ( !m_request_tracker.empty() ) {
        fprintf(fp,"Memory Sub Parition %u: pending memory requests:\n", m_id);
        for ( std::set<mem_fetch*>::const_iterator r=m_request_tracker.begin(); r != m_request_tracker.end(); ++r ) {
            mem_fetch *mf = *r;
            if ( mf )
                mf->print(fp);
            else
                fprintf(fp," <NULL mem_fetch?>\n");
        }
    }
    if( !m_config->m_L2_config.disabled() )
       m_L2cache->display_state(fp);
}

void memory_stats_t::visualizer_print( gzFile visualizer_file )
{
   gzprintf(visualizer_file, "Ltwowritemiss: %d\n", L2_write_miss);
   gzprintf(visualizer_file, "Ltwowritehit: %d\n",  L2_write_hit);
   gzprintf(visualizer_file, "Ltworeadmiss: %d\n", L2_read_miss);
   gzprintf(visualizer_file, "Ltworeadhit: %d\n", L2_read_hit);
   clear_L2_stats_pw();

   if (num_mfs)
      gzprintf(visualizer_file, "averagemflatency: %lld\n", mf_total_lat/num_mfs);
}

void memory_stats_t::clear_L2_stats_pw(){
    L2_write_miss = 0;
    L2_write_hit = 0;
    L2_read_miss = 0;
    L2_read_hit = 0;
}

void gpgpu_sim::print_dram_stats(FILE *fout) const
{
	unsigned cmd=0;
	unsigned activity=0;
	unsigned nop=0;
	unsigned act=0;
	unsigned pre=0;
	unsigned rd=0;
	unsigned wr=0;
	unsigned req=0;
	unsigned tot_cmd=0;
	unsigned tot_nop=0;
	unsigned tot_act=0;
	unsigned tot_pre=0;
	unsigned tot_rd=0;
	unsigned tot_wr=0;
	unsigned tot_req=0;

	for (unsigned i=0;i<m_memory_config->m_n_mem;i++){
		m_memory_partition_unit[i]->set_dram_power_stats(cmd,activity,nop,act,pre,rd,wr,req);
		tot_cmd+=cmd;
		tot_nop+=nop;
		tot_act+=act;
		tot_pre+=pre;
		tot_rd+=rd;
		tot_wr+=wr;
		tot_req+=req;
	}
    fprintf(fout,"gpgpu_n_dram_reads = %d\n",tot_rd );
    fprintf(fout,"gpgpu_n_dram_writes = %d\n",tot_wr );
    fprintf(fout,"gpgpu_n_dram_activate = %d\n",tot_act );
    fprintf(fout,"gpgpu_n_dram_commands = %d\n",tot_cmd);
    fprintf(fout,"gpgpu_n_dram_noops = %d\n",tot_nop );
    fprintf(fout,"gpgpu_n_dram_precharges = %d\n",tot_pre );
    fprintf(fout,"gpgpu_n_dram_requests = %d\n",tot_req );
}

unsigned memory_sub_partition::flushL2() 
{ 
    if (!m_config->m_L2_config.disabled()) {
        m_L2cache->flush(); 
    }
    return 0;   //TODO: write the flushed data to the main memory
}

unsigned memory_sub_partition::invalidateL2()
{
    if (!m_config->m_L2_config.disabled()) {
        m_L2cache->invalidate();
    }
    return 0;
}

bool memory_sub_partition::busy() const 
{
    return !m_request_tracker.empty();
}

std::vector<mem_fetch*> memory_sub_partition::breakdown_request_to_sector_requests(mem_fetch* mf)
{
	std::vector<mem_fetch*> result;

	if(mf->get_data_size() == SECTOR_SIZE && mf->get_access_sector_mask().count() == 1) {
		result.push_back(mf);
	} else if (mf->get_data_size() == 128 || mf->get_data_size() == 64) {
        //We only accept 32, 64 and 128 bytes reqs
		unsigned start=0, end=0;
		if(mf->get_data_size() == 128) {
			start=0; end=3;
		} else if (mf->get_data_size() == 64 && mf->get_access_sector_mask().to_string() == "1100") {
			start=2; end=3;
		} else if (mf->get_data_size() == 64 && mf->get_access_sector_mask().to_string() == "0011") {
			start=0; end=1;
		} else if (mf->get_data_size() == 64 && (mf->get_access_sector_mask().to_string() == "1111" || mf->get_access_sector_mask().to_string() == "0000")) {
			if(mf->get_addr() % 128 == 0) {
				start=0; end=1;
			} else {
				start=2; end=3;
			}
		} else
			{
			    printf("Invalid sector received, address = 0x%06x, sector mask = %s, data size = %d",
			    		mf->get_addr(), mf->get_access_sector_mask(), mf->get_data_size());
				assert(0 && "Undefined sector mask is received");
			}

		std::bitset<SECTOR_SIZE*SECTOR_CHUNCK_SIZE> byte_sector_mask;
		byte_sector_mask.reset();
		for(unsigned  k=start*SECTOR_SIZE; k< SECTOR_SIZE; ++k)
			byte_sector_mask.set(k);

		for(unsigned j=start, i=0; j<= end ; ++j, ++i){

			const mem_access_t *ma = new  mem_access_t( mf->get_access_type(),
									mf->get_addr() + SECTOR_SIZE*i,
									SECTOR_SIZE,
									mf->is_write(),
									mf->get_access_warp_mask(),
									mf->get_access_byte_mask() & byte_sector_mask,
									std::bitset<SECTOR_CHUNCK_SIZE>().set(j));

			 mem_fetch *n_mf = new mem_fetch( *ma,
								NULL,
								mf->get_ctrl_size(),
								mf->get_wid(),
								mf->get_sid(),
								mf->get_tpc(),
								mf->get_mem_config(),
								mf);

			 result.push_back(n_mf);
			 byte_sector_mask <<= SECTOR_SIZE;
		}
	} else {
		 printf("Invalid sector received, address = 0x%06x, sector mask = %d, byte mask = , data size = %d",
					    		mf->get_addr(), mf->get_access_sector_mask().count(), mf->get_data_size());
		 assert(0 && "Undefined data size is received");
	}

	return result;
}

void memory_sub_partition::push( mem_fetch* m_req, unsigned long long cycle )
{
    if (m_req) {
    	m_stats->memlatstat_icnt2mem_pop(m_req);
    	std::vector<mem_fetch*> reqs;
    	if(m_config->m_L2_config.m_cache_type == SECTOR)
    		reqs = breakdown_request_to_sector_requests(m_req);
    	else
    		reqs.push_back(m_req);

    	for(unsigned i=0; i<reqs.size(); ++i) {
    		mem_fetch* req = reqs[i];
			m_request_tracker.insert(req);
			if( req->istexture() ) {
				m_icnt_L2_queue->push(req);
				req->set_status(IN_PARTITION_ICNT_TO_L2_QUEUE,gpu_sim_cycle+gpu_tot_sim_cycle);
			} else {
				rop_delay_t r;
				r.req = req;
				r.ready_cycle = cycle + m_config->rop_latency;
				m_rop.push(r);
				req->set_status(IN_PARTITION_ROP_DELAY,gpu_sim_cycle+gpu_tot_sim_cycle);
			}
    	}
    }
}

mem_fetch* memory_sub_partition::pop() 
{
    mem_fetch* mf = m_L2_icnt_queue->pop();
    m_request_tracker.erase(mf);
    if ( mf && mf->isatomic() )
    //mf->do_atomic();
    if( mf && (mf->get_access_type() == L2_WRBK_ACC || mf->get_access_type() == L1_WRBK_ACC) ) {
        delete mf;
        mf = NULL;
    } 
    return mf;
}

mem_fetch* memory_sub_partition::top() 
{
    mem_fetch *mf = m_L2_icnt_queue->top();
    if( mf && (mf->get_access_type() == L2_WRBK_ACC || mf->get_access_type() == L1_WRBK_ACC) ) {
        m_L2_icnt_queue->pop();
        m_request_tracker.erase(mf);
        delete mf;
        mf = NULL;
    } 
    return mf;
}

void memory_sub_partition::set_done( mem_fetch *mf )
{
    m_request_tracker.erase(mf);
}

void memory_sub_partition::accumulate_L2cache_stats(class cache_stats &l2_stats) const {
    if (!m_config->m_L2_config.disabled()) {
        l2_stats += m_L2cache->get_stats();
    }
}

void memory_sub_partition::get_L2cache_sub_stats(struct cache_sub_stats &css) const{
    if (!m_config->m_L2_config.disabled()) {
        m_L2cache->get_sub_stats(css);
    }
}

void memory_sub_partition::get_L2cache_sub_stats_pw(struct cache_sub_stats_pw &css) const{
    if (!m_config->m_L2_config.disabled()) {
        m_L2cache->get_sub_stats_pw(css);
    }
}

void memory_sub_partition::clear_L2cache_stats_pw() {
    if (!m_config->m_L2_config.disabled()) {
        m_L2cache->clear_pw();
    }
}

void memory_sub_partition::visualizer_print( gzFile visualizer_file )
{
    // Support for L2 AerialVision stats
    // Per-sub-partition stats would be trivial to extend from this
    cache_sub_stats_pw temp_sub_stats;
    get_L2cache_sub_stats_pw(temp_sub_stats);

    m_stats->L2_read_miss += temp_sub_stats.read_misses;
    m_stats->L2_write_miss += temp_sub_stats.write_misses;
    m_stats->L2_read_hit += temp_sub_stats.read_hits;
    m_stats->L2_write_hit += temp_sub_stats.write_hits;

    clear_L2cache_stats_pw();
}

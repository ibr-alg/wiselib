
#ifndef SHAWN_TAG_NEIGHBORHOOD_H
#define SHAWN_TAG_NEIGHBORHOOD_H

#include "util/pstl/vector_static.h"

#include "sys/taggings/basic_tags.h"
#include "sys/world.h"

namespace wiselib {

template<
   typename OsModel_P,
   typename ShawnRadio_P
>
class ShawnTagNeighborhood {
   public:
      typedef OsModel_P OsModel;
      typedef ShawnRadio_P Radio;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename OsModel::size_t size_t;
      typedef typename OsModel::block_data_t block_data_t;
      typedef ShawnTagNeighborhood<OsModel, Radio> self_type;
      typedef self_type* self_pointer_t;
      
      enum { MAX_NODES = 16 };
      
      typedef vector_static<OsModel, node_id_t, MAX_NODES> Neighbors;
      
      int init(typename Radio::self_pointer_t radio) {
         radio_ = radio;
         return init();
      }
      
      int init() {
         is_root_ = true;

         neighbors_.clear();
         
         typedef typename shawn::World::node_iterator iter_t;
         typename shawn::World& world = radio_->os().proc->owner_w().world_w();
         
         for(iter_t iter = world.begin_nodes_w(); iter != world.end_nodes_w(); ++iter) {
            shawn::TagHandle th = iter->find_tag_w("predecessor");
            
            if(th.get()) {
               shawn::StringTag* t = dynamic_cast<shawn::StringTag*>(th.get());

               shawn::TagHandle th_addr = iter->find_tag_w("radio_id");
               if(th_addr.get()) {
                  shawn::IntegerTag* t_addr = dynamic_cast<shawn::IntegerTag*>(th_addr.get());

                  // parent of our node
                  if (t_addr->value() == radio_->id()){
                     parent_ = atoi(t->value().c_str());
                     is_root_ = false;
                  }

                  // if the node has a parent-tag pointing to us
                  // and if the node has a radio_id
                  if(atoi(t->value().c_str()) == radio_->id()) {
                     neighbors_.push_back(t_addr->value());
                  }
               }
            }
         }
         
         return Os::SUCCESS;
      }
      
      int destruct() {
         return Os::SUCCESS;
      }
      
      void enable() {
      }
      
      void disable() {
      }
      
      Neighbors& topology() {
         return neighbors_;
      }

      node_id_t parent(){
         return parent_;
      }

      bool is_root(){
         return is_root_;
      }

   private:
      Neighbors neighbors_;
      node_id_t parent_;
      bool is_root_;
      typename Radio::self_pointer_t radio_;
};

}

#endif // SHAWN_TAG_NEIGHBORHOOD_H
/* vim: set ts=3 sw=3 tw=78 expandtab :*/

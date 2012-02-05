#ifndef WISELIB_CONCEPT_CHECK_RADIO
#define WISELIB_CONCEPT_CHECK_RADIO
// vim: ts=3:sw=3

namespace wiselib
{
namespace concept_check
{

template <class X>
struct Radio
{
   public:
      typedef typename X::OsModel         OsModel;
      typedef typename X::self_pointer_t  self_pointer_t;
      typedef typename X::node_id_t       node_id_t;
      typedef typename X::block_data_t    block_data_t;
      typedef typename X::size_t          size_t;
      typedef typename X::message_id_t    message_id_t;

      typedef typename X::SpecialNodeIds  SpecialNodeIds;
      typedef typename X::Restrictions    Restrictions;

      BOOST_CONCEPT_USAGE(Radio)
      {
         same_type(sni, X::BROADCAST_ADDRESS);
         same_type(sni, X::NULL_NODE_ID);
         same_type(res, X::MAX_MESSAGE_LENGTH);

         same_type(val_int, radio.enable_radio());
         same_type(val_int, radio.disable_radio());

         same_type(val_int, radio.send(node_id, size, &block_data));
         same_type(val_int, radio.id());

         same_type(val_int, radio.template reg_recv_callback<callback, &callback::method>(&callback_obj));
         same_type(val_int, radio.unreg_recv_callback(val_int));
      }

   private:
      X radio;

      SpecialNodeIds sni;
      Restrictions res;

      int val_int;

      self_pointer_t self_pointer;
      node_id_t node_id;
      block_data_t block_data;
      size_t size;
      message_id_t message_id;

      struct callback {
         void method(node_id_t node_id, size_t size, block_data_t *block_data) {}
      };
      callback callback_obj;

      template <typename T>
      void same_type(const T&, const T&);
};

}
}

#endif  // WISELIB_CONCEPT_CHECK_RADIO

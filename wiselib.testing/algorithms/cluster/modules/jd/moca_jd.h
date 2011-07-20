#ifndef _MOCA_JD_H
#define	_MOCA_JD_H

namespace wiselib {

    /**
     * \ingroup jd_concept
     * 
     * Moca join decision module.
     */
    template<typename OsModel_P>

    class MocaJoinDecision {
    public:

        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Debug Debug;


        // data types
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef node_id_t cluster_id_t;

        /*
         * Constructor
         * */
        MocaJoinDecision() :
        id_(0),
        maxhops_(0) {
        };

        /*
         * Destructor
         * */
        ~MocaJoinDecision() {
        };

        /*
         * INIT
         * initializes the values of radio and debug
         */
        void init(Radio& radio, Debug& debug) {
            radio_ = &radio;
            debug_ = &debug;
        };


        /* SET functions */

        //set the id

        void set_id(node_id_t id) {
            id_ = id;
        };

        void set_maxhops(int maxhops){
            maxhops_ = maxhops;
        }

        
        /* GET functions */

        //get the CH_ADVERTISE payload
        
        void get_join_request_payload(block_data_t * mess) {

            //block_data_t ret[ get_payload_length(JOIN) ];

            uint8_t type = JOIN;
            memcpy(mess,&type,sizeof(uint8_t));
//            ret[0] = JOIN; // type of message

            memcpy(mess+sizeof(uint8_t),&id_,sizeof(cluster_id_t));
//            ret[1] = cluster_id_ % 256; // cluster_id
//            ret[2] = cluster_id_ / 256;

//            ret[3] = hops_ + 1; // hops
            uint8_t now_hops = 1;
            memcpy(mess+sizeof(uint8_t)+sizeof(cluster_id_t),&now_hops,sizeof(uint8_t));

            //memcpy(mess, ret, get_payload_length(JOIN));
        };

        size_t get_payload_length(int type) {
            if (type == JOIN)
                return 1+sizeof(cluster_id_t)+1;
            else
                return 0;
        };


        /*
         * JOIN
         * respond to an JOIN message received
         * either join to a cluster or not
         * */
        bool join(uint8_t *payload, uint8_t length) {
            //copy message to local memory

            uint8_t hops;
            memcpy(&hops, payload+1+sizeof(cluster_id_t),sizeof(uint8_t));          

            //if cluser is close
            if (hops <= maxhops_) {
                //join the cluster
                //return true
                return true;
            }//if cluster is away
            else {
                //return false
                return false;
            }
        };

        /*
         * ENABLE
         * enables the module
         * initializes values
         * */
        void enable() {
            id_ = -1;
            maxhops_ = 0;
        };

        /*
         * DISABLE
         * disables this bfsclustering module
         * unregisters callbacks
         * */
        void disable() {
        };


    private:
        node_id_t id_; //the node's id
        int maxhops_; //hops from cluster head

        Radio * radio_; //radio module
        Debug * debug_; //debug module

        Radio& radio() {
            return *radio_;
        }

        Debug& debug() {
            return *debug_;
        }

    };
}


#endif


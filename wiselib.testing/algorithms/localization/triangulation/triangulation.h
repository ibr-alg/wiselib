#ifndef __TRIANGULATION_H__
#define __TRIANGULATION_H__

#define DEBUG_TRIANGULATION

//#include "algorithms/routing/routing_base.h"
#include "algorithms/localization/triangulation/triangulation_message.h"
#include "algorithms/localization/localization_base.h"
#include "util/delegates/delegate.hpp"
//----------------------------------------------------
#include <math.h>
//#include <cmath>
#include <stdlib.h>
#include <time.h>
#include "internal_interface/routing_table/routing_table_static_array.h"
//----------------------------------------------------
int __errno;
//----------------------------------------------------

namespace wiselib
{

    /** \brief triangulation algorithm implementation
    */


   template<typename OsModel_P,
            typename Radio_P,
            typename Distance_P,
            typename Debug_P = typename OsModel_P::Debug>
   class Triangulation
//	   : public LocalizationBase<OsModel_P,Radio_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;
      typedef Distance_P Distance;

      typedef StaticArrayRoutingTable <OsModel, Radio, 15, int> int_map_t;
      typedef typename int_map_t::iterator int_map_iterator_t;


      typedef Triangulation<OsModel, Radio, Distance, Debug> self_type;

      typedef typename OsModel::Timer Timer;
      typedef typename Timer::millis_t millis_t;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;

      typedef TriangulationMessage<OsModel, Radio> Message;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL
      };
      // --------------------------------------------------------------------
      ///@name Construction / Destruction
      ///@{
      Triangulation();
      ~Triangulation();
      ///@}

      ///@name Enable / Disable
      ///@{
      void enable( void );
      void disable( void );
      ///@}

      ///@name Algorithm Initialization
      ///@{
      void send(  );
      ///@}

      ///@name Methods called by RadioModel
      ///@{
      void receive( node_id_t from, size_t len, block_data_t *data );
      ///@}

      /**
       * \brief Returns the internal index of a node ID.
       * \param ID node ID the index is requested for
       * \return Corresponding index
       */
      int getIndex(node_id_t);

      /**
       * \brief Returns the node ID that is mapped to a internal index.
       * \param index Internal index that identifies the corresponding node ID
       * \return Corresponding node ID
       */
      int getNode (int);

      /**
       * \brief Identifies two nodes p and q that help finding coordinates of other nodes.
       */
      void findP();

      /**
       * \brief Uses the identified nodes p and q to find coordinates of other nodes.
       * The function is called by findP().
       */
      void findCoordinatesP();

      /**
       * \brief Identifies two nodes p and q that help finding coordinates of other nodes.
       * Requires to adjust a tolerance parameter that is located inside the method.
       * The function has not been tested on actual nodes!
       */
      void findPQ();

      /**
       * \brief Uses two nodes p and q to find coordinates of other nodes.
       * The function is called by findPQ().
       * The function has not been tested on actual nodes!
       */
      void findCoordinatesPQ();

      /**
       * \brief Iterates over all found coordinates and tries to find potential triangles.
       */
      void findCircle();

      /**
       * \brief Determines whether a circle of three nodes contains a node inside.
       * \param firstID ID of the first node
       * \param secondID ID of the second node
       * \return Integer value that represents the result.
       */
      int testCircle(int, int);

      /**
       * \brief Function that assigns other nodes ratings to found circles.
       * \param senderID ID of the sender
       * \param thirdID ID of the third node being part of the triangle
       * \param rating Rating the sender transmitted
       */
      void setRating(node_id_t, node_id_t, int);	//Absender-ID, 3. Dreiecks-ID, Bewertung

      /**
       * \brief Converts a found circle and therefore potential triangle to a mandatory global triangle.
       * \param ownID The ID of the node itself
       * \param firstID First ID of found circle
       * \param secondID Second ID of found circle
       * \param trust Trust value of the triangle
       *
       */
      void setTriangleGlobal(node_id_t, node_id_t, node_id_t, int);

      /**
       * \brief Checks whether the node is part of at least one triangle.
       * \return Boolean value that represents the result
       */
      bool nodeInTriangles();

      /**
       * \brief Returns the trust value corresponding to two ratings from two nodes.
       * \param firstRating First rating value
       * \param secondRating Second rating value
       * \return Trust value
       */
      int lookUpTrust(int, int);

      /**
       * \brief Iterates over all known circles and returns the one with the highest rating that incorporates a specific node ID
       * \param nodeID Node ID that a circle is searched for
       * \return Index of the found circle.
       */
      int bestCircleIndex(node_id_t);

      /**
       * \brief Returns the maximum of two integer values.
       */
      int max(int,int);

      /**
       * \brief Returns the minimum of two integer values.
       */
      int min(int,int);

      /**
       * \brief Timer that starts the determination of coordinates.
       */
      void coordinates_timer_elapsed( void *userdata );

      /**
       * \brief Timer that checks whether the node is part of a triangle and then tries to link the node.
       */
      void complete_triangulation_timer_elapsed( void *userdata);

      /**
       * \brief Timer that selects the best triangle proposal on expiration.
       */
      void select_best_circle_timer_elapsed( void *userdata);

      /**
       * \brief Timer that prints out some results.
       */
      void fourth_timer_elapsed( void *userdata);

      /**
       * \brief Timer that calls the send function.
       */
      void send_timer_elapsed( void *userdata);

      /**
       * \brief Timer that will send out the next message from the message vector.
       */
      void message_timer_elapsed (void *userdata);

     // void reset_data();

      void triangles_changed(int);

      typedef delegate1<void, int> localization_delegate_t;

      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(int)>
      inline void reg_changed_callback( T *obj_pnt )
      {
    	  callback_ = localization_delegate_t::from_method<T, TMethod>( obj_pnt );
      };
      // --------------------------------------------------------------------
      inline void unreg_changed_callback( void )
      {
    	  callback_ = localization_delegate_t();
      }
      // --------------------------------------------------------------------
      inline void notify_receivers( int value )
      {
    	  if (callback_)
    		  callback_( value );
      }

      localization_delegate_t callback_;

      int init( Radio& radio, Timer& timer, Debug& debug )
      {
         radio_ = &radio;
         timer_ = &timer;
         debug_ = &debug;
         return SUCCESS;
      }

      int init()
      { return ERR_NOTIMPL; }

      int destruct()
      { return ERR_NOTIMPL; }

   private:
      Radio& radio()
      { return *radio_; }

      Timer& timer()
      { return *timer_; }

      Debug& debug()
      { return *debug_; }

      typename Radio::self_pointer_t radio_;
      typename Timer::self_pointer_t timer_;
      typename Debug::self_pointer_t debug_;

      Distance dist_est_;


      enum MessageIds
      {
         DISTANCE_CHECK = 1,
         DISTANCE_RETURN = 2,
         DISTANCE_PROP = 3,
         CHECK_CIRCLE = 4,
         CHECK_CIRCLE_RETURN = 5,
         TRIANGLE_PROP = 6,
         ASKFORCIRCLE = 7,
         RETURN_CIRCLE = 8,
         RESET = 98,
         RESET_PROP = 99,

      };

      enum Various
      {
    	  MESSAGES_SIZE = 15,
    	  NODECOUNT = 10,
    	  VECTORSIZE = 20,		//1.5*nodecount reicht sicher
    	  MILLIS = 1000,
    	  POSITION_CHANGED = 0,
    	  UNKNOWN_POSITION = 1
      };

      short callback_id_;

      struct circle {
    	  node_id_t idFirst, idSecond;
    	  int ratingFirst, ratingSecond;
      };
      struct triangle {
    	  node_id_t idFirst, idSecond, idThird;
    	  bool prop;
    	  int trust; //1 für: alle stimmen zu, 2 für: einer siehts nicht, 3 für: zwei sehns nicht, 4 für: einer stimmt zu, einer lehnt ab, 5 für: zwei lehnen ab
      };

      typedef vector_static<OsModel,triangle,VECTORSIZE> position_t;

      inline position_t *position() {
    	  if(nodeInTriangles)
    		  return &triangles;
    	  else
    		  return NULL;
      }

      int_map_t neighbourMap;

      float distances[NODECOUNT];
      float ndistances[NODECOUNT][NODECOUNT];
      float coordinates[NODECOUNT][2]; //für alle Knoten an [i][0] x und an [i][1] y

      bool send_called;
      short neighbourCount;

      int p_index;
      int q_index;
      node_id_t demandedCircleIDs[3];

      millis_t waitingTime1;
      millis_t waitingTime2;
      millis_t startupTime;

      bool coordinates_timer_started;
      bool send_timer_iteration;
      bool coordinates_called;
      int send_timer_running;
      int complete_triangulation_timer_running;

      vector_static<OsModel, circle, VECTORSIZE> foundCircles;
      vector_static<OsModel, triangle, VECTORSIZE> triangles;
      vector_static<OsModel, Message, MESSAGES_SIZE> messages;


   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P, typename Distance_P,
            typename Debug_P>
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   Triangulation()
      : os_           ( 0 ),
        callback_id_ ( 0 ),
        send_called(false),
        coordinates_timer_started(false),
        send_timer_iteration(true),
        neighbourCount(0),
        send_timer_running(0),
        complete_triangulation_timer_running(0),
        p_index(-1), q_index(-1),
        waitingTime1(8000),
        waitingTime2(3000),
        startupTime (5000)
   {};
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P, typename Distance_P,
            typename Debug_P>
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   ~Triangulation()
   {
     // debug().debug(  "Triangulation:dtor\n" );

   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P, typename Distance_P,
            typename Debug_P>
   void
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   enable( void )
   {
#ifdef DEBUG_TRIANGULATION
      debug().debug(  "Triangulation: Boot for %i - send_timer started \n", radio().id(  ) );
#endif

      radio().enable_radio(  );
      callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>(  this );

      reg_changed_callback<self_type, &self_type::triangles_changed>(this);

      dist_est_.set_os();
      dist_est_.enable();

      neighbourMap[radio().id()] = 0;

      for(int i=0; i<NODECOUNT;i++) {
    	  distances[i] = 0;
    	  coordinates[i][0] = 0;
    	  coordinates[i][1] = 0;
    	  for(int j=0;j<NODECOUNT;j++) {
    		  ndistances[i][j]= 0;
    	  }
      }
      demandedCircleIDs[2] = -1;

	  timer().template set_timer<self_type, &self_type::send_timer_elapsed>( startupTime,this , 0 );

   }

   template<typename OsModel_P,
              typename Radio_P, typename Distance_P,
              typename Debug_P>
     void
     Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
     disable( void )
     {
        //debug().debug(  "Triangulation: Disable\n" );

        radio().unreg_recv_callback(  callback_id_ );
        radio().disable(  );
     }

   // -----------------------------------------------------------------------

     template<typename OsModel_P,
			  typename Radio_P, typename Distance_P,
			  typename Debug_P>
     void
     Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
     triangles_changed(int ID) {

    	 if(ID == POSITION_CHANGED) {
    		 debug().debug("related triangles changed for %i \n", radio().id());
    	 }

     }

     template<typename OsModel_P,
			 typename Radio_P, typename Distance_P,
			 typename Debug_P>
     int
     Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
     getIndex(node_id_t nodeID) {

    	 for(int_map_iterator_t it = neighbourMap.begin(); it != neighbourMap.end(); it++) {
    		 if(it->first == nodeID) {
    			 return it->second;
    		 }
    	 }

    	 if(neighbourMap.find(nodeID) == neighbourMap.end()) { //nodeID noch nicht gespeichert
    		 neighbourMap[nodeID] = 0;
    		 neighbourMap[nodeID] = neighbourMap.find(nodeID) - neighbourMap.begin();
    	 }

    	 return neighbourMap.find(nodeID) - neighbourMap.begin();
     }

     template<typename OsModel_P,
     typename Radio_P, typename Distance_P,
     typename Debug_P>
     int
     Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
     getNode(int abc) {

    	 for(int_map_iterator_t it = neighbourMap.begin(); it != neighbourMap.end(); it++) {
    		 if(it->second == abc) {
    			 return it->first;
    		 }
    	 }
    	 return -1;
     }

     template<typename OsModel_P,
			  typename Radio_P, typename Distance_P,
			  typename Debug_P>
     void
     Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
     coordinates_timer_elapsed (void* userdata) {

    	 send_timer_iteration = false;
    	 coordinates_called = true;

  	   if(p_index==-1 && q_index==-1) {
  		   findP();

  		   //Distanzen und p/q vorhanden, Kreise schon gesucht?
  		   if(foundCircles.size() == 0) {

  			   findCircle();

  			  timer().template set_timer<self_type, &self_type::complete_triangulation_timer_elapsed>( waitingTime1, this, 0 );
  		   }
  	   }
     }


   /* nachsehen, ob man selbst in der Triangulierung drin ist
    * wenn nicht: genau 2 nachbarn? -> dreieck einfügen
    * wenn sonst: nachricht versenden, nachbarn dreiecke suchen lassen
    */
   template<typename OsModel_P,
               typename Radio_P, typename Distance_P,
               typename Debug_P>
   void
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   complete_triangulation_timer_elapsed(void* userdata) {

#ifdef DEBUG_TRIANGULATION
debug().debug( "complete_triangulation_timer \n");
#endif

    	  if(!nodeInTriangles()) {
#ifdef DEBUG_TRIANGULATION
    		  debug().debug( "nicht in triangles: %i \n", radio().id());
#endif

    		  for(int i=0;i<NODECOUNT;i++) {
    			  if(distances[i] != 0) neighbourCount++;
    		  }

    		  if(neighbourCount == 2) {
    			  int neighbour_indices[2] = {-1,-1};
    			  for(unsigned int i=0; i<neighbourMap.size(); i++) {
    				  if(distances[i] != 0) {
    					  if(neighbour_indices[0] == -1) neighbour_indices[0] = i;
    					  else neighbour_indices[1]=i;
    				  }
    			  }

    			  if(getNode(neighbour_indices[0]) < getNode(neighbour_indices[1]))
    				  setTriangleGlobal(radio().id(),getNode(neighbour_indices[0]),getNode(neighbour_indices[1]),2);
    			  else  setTriangleGlobal(radio().id(),getNode(neighbour_indices[1]),getNode(neighbour_indices[0]),2);

#ifdef DEBUG_TRIANGULATION
    			  debug().debug( "2 nachbarn, dreieck eingefuegt \n");
#endif
    		  }
    		  else {	//nicht genau 2 nachbarn
    			  Message message;
    			  message.set_msg_id(ASKFORCIRCLE);
    			  message.destination = radio().BROADCAST_ADDRESS;
    			  //radio().send(radio().BROADCAST_ADDRESS,message.buffer_size(), (block_data_t*)&message);

    			  messages.push_back(message);
    			  millis_t random = rand();
    			  random = random % MILLIS;
    			  timer().template set_timer<self_type, &self_type::message_timer_elapsed>( random ,this , 0 );

    			  demandedCircleIDs[2] = -1;
    		  }

    		  timer().template set_timer<self_type, &self_type::select_best_circle_timer_elapsed>( waitingTime2, this, 0 );
    	  }

    	  //*in* triangles, auswertung starten, und doppelte zeit warten (select_best überspringen)
    	  else {
    		  timer().template set_timer<self_type, &self_type::fourth_timer_elapsed>( waitingTime2+waitingTime2, this, 0);
    	  }
   }

   /* Auswerten der Ergebnisse von Vorschlägen für Kreise
    *
    */
   template<typename OsModel_P,
                  typename Radio_P, typename Distance_P,
                  typename Debug_P>
   void
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   select_best_circle_timer_elapsed (void* userdata) {

	   //für Auswertung
	   timer().template set_timer<self_type, &self_type::fourth_timer_elapsed>( waitingTime2, this, 0);

	   node_id_t selfID = radio().id();

	   //keine Antwort von Nachbarn, dann selbst suchen
	   if(demandedCircleIDs[2] == -1) {
#ifdef DEBUG_TRIANGULATION
		  debug().debug( "keine antwort, eigener bestCircle \n");
#endif
		  int index = bestCircleIndex(selfID);
		   if(index>=0) {
			   setTriangleGlobal(selfID,foundCircles[index].idFirst,foundCircles[index].idSecond, lookUpTrust(foundCircles[index].ratingFirst, foundCircles[index].ratingSecond));

#ifdef DEBUG_TRIANGULATION
			   debug().debug( "Rating: %i %i \n",foundCircles[index].ratingFirst,foundCircles[index].ratingSecond);
			   debug().debug( "IDs: %i %i \n", foundCircles[index].idFirst, foundCircles[index].idSecond);
#endif

		   }
	   }
	   //knoten noch ohne anschluss, hat aber vorschläge erhalten
	   else if(demandedCircleIDs[2] != -1 && demandedCircleIDs[2] != 0) {

#ifdef DEBUG_TRIANGULATION
		   debug().debug( "Kreisvorschlag mit Trust: %i \n", demandedCircleIDs[2]);
		   debug().debug( "IDs: %i und %i \n", demandedCircleIDs[0],demandedCircleIDs[1]);
#endif

		   //prüfen, ob ein eigener kreis besser ist als der vorgeschlagene
		   int index = bestCircleIndex(selfID);
		   if(lookUpTrust(foundCircles[index].ratingFirst, foundCircles[index].ratingSecond) < demandedCircleIDs[2])
			   setTriangleGlobal(selfID,foundCircles[index].idFirst,foundCircles[index].idSecond, lookUpTrust(foundCircles[index].ratingFirst, foundCircles[index].ratingSecond));
		   else
			   setTriangleGlobal(selfID,demandedCircleIDs[0],demandedCircleIDs[1],demandedCircleIDs[2]);
	   }
   }

	template<typename OsModel_P,
			 typename Radio_P, typename Distance_P,
			 typename Debug_P>
	void
	Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
	fourth_timer_elapsed (void* userdata) {

#ifdef DEBUG_TRIANGULATION
		 debug().debug( "distances-array: \n");
		 for(int i=0;i<NODECOUNT;i++) {
			 if(distances[i] != 0) {
				 debug().debug( "distance[%i]: %i (node %i) \n",i,(int)distances[i],getNode(i));
			 }
		 }

		 debug().debug( "ndistances: \n");
		 for(int i=0;i<NODECOUNT;i++) {
			 for(int j=0; j<NODECOUNT; j++) {
				 if(ndistances[i][j] != 0) {
					 debug().debug( "ndistances[%i][%i]: %i \n", i,j,(int)ndistances[i][j]);
				 }
			 }
		 }

		debug().debug("messages.size()=%i \n", messages.size());

		debug().debug( "foundCircles.size()=%i \n", foundCircles.size());
		debug().debug( "triangles.size()=%i \n", triangles.size());

		for(int i=0;i<foundCircles.size();i++) {
			debug().debug( "foundCircles[%i].idFirst = %i, idSecond = %i \n",i,foundCircles[i].idFirst, foundCircles[i].idSecond);
			debug().debug( "foundCircles[%i].ratingFirst = %i, ratingSecond = %i \n", i, (int)foundCircles[i].ratingFirst, (int)foundCircles[i].ratingSecond);
		}

		for(int i=0;i<triangles.size();i++) {
			debug().debug( "triangles[%i].idFirst = %i, idSecond = %i,idThird=%i \n",i,triangles[i].idFirst, triangles[i].idSecond, triangles[i].idThird);
		}
#endif

	}

	template<typename OsModel_P,
	typename Radio_P, typename Distance_P,
	typename Debug_P>
	void
	Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
	message_timer_elapsed (void* userdata) {

		Message message;
		message = messages[0];
		node_id_t destination = messages[0].destination;
		radio().send(destination,message.buffer_size(),(block_data_t*)&message);
		debug().debug("msg sent->%i - id:%i \n", (int)destination, (int)messages[0].msg_id());
		messages.erase(messages.begin());

	}


	template<typename OsModel_P,
			 typename Radio_P, typename Distance_P,
			 typename Debug_P>
	void
	Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
	send_timer_elapsed (void* userdata) {

#ifdef DEBUG_TRIANGULATION
		//debug().debug( "send_timer elapsed, rufe send() auf \n");
#endif

		send_timer_running = 0;
		if(coordinates_called = true && send_timer_running ==  0) {
			timer().template set_timer<self_type, &self_type::send_timer_elapsed>( 500, this, 0);
			send_timer_running = 1;
		}

		send();
	}
	// -----------------------------------------------------------------------

	/*template<typename OsModel_P,
			 typename Radio_P, typename Distance_P,
			 typename Debug_P>
	void Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::reset_data() {

		send_called = false;
		p_index = -1;
		q_index = -1;
		neighbourCount = 0;
		neighbourMap.clear();
		foundCircles.clear();
		triangles.clear();

	}*/

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
                 typename Radio_P, typename Distance_P,
                 typename Debug_P>
   int Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::max(int a, int b) {
	   return (a>b?a:b);
   }

   template<typename OsModel_P,
                    typename Radio_P, typename Distance_P,
                    typename Debug_P>
   int Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::min(int a, int b) {
	   return (a<b?a:b);

   }

   template<typename OsModel_P,
              typename Radio_P, typename Distance_P,
              typename Debug_P>
   void
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   findP() {

#ifdef DEBUG_TRIANGULATION
		   debug().debug( "findP \n");
#endif

	   //Sucht einen Knoten, der möglichst viele der eigenen Nachbarn erreicht
	 int commonNeighbours = 0;
	 int commonNeighboursBest = 0;
	 node_id_t nodesID_index = 0;

	 //Gehe gesamte Matrix durch
	 for(int i=0;i<NODECOUNT;i++) {

		 //Zwischenspeichern des besten Ergebnisses
		 if(commonNeighboursBest < commonNeighbours) {
			 commonNeighboursBest = commonNeighbours;
			 nodesID_index = i-1;
		 }

		 //Gleichheit der Ergebnisse -> zufällig q tauschen
		/* if(commonNeighboursBest == commonNeighbours) {
			 double random1 = rand()%10;	//random1 von 0..9
			 if(random1<4) {				//random1 in 0..3 ? Tauschen
				 nodesID_index = i-1;
			 }
		 }*/

		 //Zurücksetzen der hilfsvariablen
		 commonNeighbours = 0;

		 if(distances[i] == 0 || i==getIndex(radio().id())) {}			//geprüfter knoten muss nachbar sein
		 else {
			 for(int j=0;j<NODECOUNT;j++) {
				 if(ndistances[i][j] != 0	//geprüfter knoten hat einen nachbarn ..
				 && distances[j] != 0		//den man auch selber sieht
				 && i != j					//nachbar des knotens ist er nicht selbst
				 //&& j !=getIndex(radio().id())) 	//nachbar ist man auch nicht selbst
				 &&j != 0)		//nachbar ist man nicht selber - man steht auf index 0
				 {
					 commonNeighbours++;
				 } //if
			 } //for j
		 } //else
	 } //for i

	 //p setzen, q wählen
	 //q so wählen, dass q sein gamma in {-1,1} liegt
	 if(commonNeighboursBest >0 ) {
		 p_index= nodesID_index;
		 q_index= -1;
		 for(int c=0;c<NODECOUNT;c++) {

			 //prüfen, ob c als potenzielles q infrage kommt - gamma muss zwischen -1 und 1 sein
			 float dip = distances[p_index];
			 float diq = distances[c];
			 float dpq = ndistances[p_index][c];
			 double gammaValue = ((diq*diq) + (dip*dip) - (dpq*dpq)) / (2*diq*dip);

			 //prüfen, ob Knoten p Knoten c sieht, und ob gamma passt
			 if(ndistances[p_index][c] != 0 && -1 <= gammaValue && gammaValue <= 1) {
				 //if(q_index==-1)
					 q_index=c;
				/* double r = rand() % 10;
				 if(r<4) q_index=c;*/
			 }
		 }
	 }
	 else {p_index=-1;q_index=-1;}

	// debug().debug( "p_index: %i->Node %i, q_index: %i->Node %i \n", p_index, getNode(p_index), q_index, getNode(q_index));

	 if(p_index!=-1 && q_index!= -1 && coordinates[p_index][1] == 0 && coordinates[q_index][0] == 0 && coordinates[q_index][1] == 0) {
			 findCoordinatesP();
	    }

   }


   template<typename OsModel_P,
                    typename Radio_P, typename Distance_P,
                    typename Debug_P>
   void
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   findCoordinatesP() {

#ifdef DEBUG_TRIANGULATION
debug().debug( "findCoordinatesP \n" );
#endif

	   int selfIndex = getIndex(radio().id());
	   coordinates[selfIndex][0] = 0;				//eigenes x: 0
	   coordinates[selfIndex][1] = 0;				//eigenes y: 0
	   coordinates[p_index][0] = distances[p_index];		//p: x entspricht abstand
	   coordinates[p_index][1] = 0;					//p: auf x-achse

	   //q-koordinaten
	//   double gamma = 0.0;
	   double dip = distances[p_index];				//distanz i->p
	   double diq = distances[q_index];				//distanz i->q
	   double dpq = ndistances[p_index][q_index];			//distanz p->q
	   double gammaValue = ((diq*diq) + (dip*dip) - (dpq*dpq)) / (2*diq*dip);
	 //  gamma = acos(gammaValue);
	 //  coordinates[q_index][0] = diq * (cos(gamma));
	 //  coordinates[q_index][1] = diq * (sin(gamma));
	   coordinates[q_index][0] = diq * (gammaValue);					//cos(arccos(x)) = x
	   coordinates[q_index][1] = diq * sqrt(1-(gammaValue*gammaValue)); //sin(arccos(x)) = sqrt(1-x*x)

	   //weitere Knoten
	  // double alpha = 0.0;

	   for(int j=0;j<NODECOUNT;j++) {
	  		   if(ndistances[p_index][j] != 0		//p hat j als nachbarn
	  		   && distances[j] != 0			//knoten sieht j auch
	  		   && j != selfIndex					//j ist man auch nicht selber
	  		   && j != p_index					//j ist auch nicht p
	  		   && j != q_index) 					//und auch nicht q
	  		   {
	  			 float dij = distances[j];					//distanz i->j
	  			 float dpj = ndistances[p_index][j];				//distanz p->j
	  			 double alphaValue = ((dij*dij) + (dip*dip) - (dpj*dpj)) / (2*dij*dip);
	  			 if(-1 <= alphaValue && alphaValue <= 1) {
	  				// alpha = acos(alphaValue);
	  				// coordinates[j][0] = dij * (cos(alpha));	//x-koordinate von knoten j
	  				 //betrag der y-koordinate von j ...
	  				// coordinates[j][1] = dij * (sin(alpha));

	  				coordinates[j][0] = dij * alphaValue;
	  				coordinates[j][1] = dij * sqrt(1-(alphaValue*alphaValue));

	  				 //Distanz von q zu beiden möglichen y-koordinaten: y1 = positiv, y2=negativ

	  				 double x = (coordinates[q_index][0] - coordinates[j][0])*(coordinates[q_index][0] - coordinates[j][0]);	//(x-x0)*(x-x0)
	  				 double y1 =(coordinates[q_index][1] - coordinates[j][1])*(coordinates[q_index][1] - coordinates[j][1]);	//(y-y1)*(y-y1)
	  				 double y2 =(coordinates[q_index][1] + coordinates[j][1])*(coordinates[q_index][1] + coordinates[j][1]);	//(y-y2)*(y-y2)

	  				 double dqjCS1 = sqrt(x + y1);	//berechnete Distanz q->j mit j positiv
	  				 double dqjCS2 = sqrt(x + y2);	//berechnete Distanz q->j mit j negativ

	  				 if(ndistances[q_index][j] != 0) {
	  					 //für schwankende abstände
	  					 double diffPos = fabs(ndistances[q_index][j] - dqjCS1);	//Differenzbetrag Messung & Berechnung für j positiv
	  					 double diffNeg = fabs(ndistances[q_index][j] - dqjCS2);	//Differenzbetrag Messung & Berechnung für j negativ
	  					 if(diffPos < diffNeg) {}	//j bleibt positiv
	  					 else if(diffPos > diffNeg) {coordinates[j][1] = -(coordinates[j][1]);} //j wird negativ
					}
	  				 else {												//Distanz q->j gibts nicht, d.h. j ganz weit weg
	  					 if(coordinates[q_index][1] > 0)				//q über der x-Achse ..
	  						 coordinates[j][1] = -(coordinates[j][1]);	//dann ist j drunter
	  					 //else implizit behandelt
	  				 } //else
	  			 } // if alpha
	  		   } // if knoten hinzufügbar
	   } // for j

   } //Funktion

   template<typename OsModel_P,
                  typename Radio_P, typename Distance_P,
                  typename Debug_P>
      void
      Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
      findPQ() {

   	   int pq[3];	//0: P, 1: Q, 2: Score
   	   pq[2] = 0; pq[1] = -1; pq[0] = -1;
   	   int pqBest[3];
   	   pqBest[2] = 0; pqBest[1] = -1; pqBest[0] = -1;

   	   for(int i=0; i<NODECOUNT; i++) {

   		   //beste pq-Kombi speichern
   		   if(pqBest[2] < pq[2]) {
   			   pqBest[0] = pq[0];
   			   pqBest[1] = pq[1];
   			   pqBest[2] = pq[2];
   		   }
   		  /* if(pqBest[2] == pq[2]) {
   			   double random1 = rand()%10;	//random1 von 0..9
   			   if(random1<4) {				//random1 in 0..3 ? Tauschen
   				   pqBest[0] = pq[0];
   				   pqBest[1] = pq[1];
   				   pqBest[2] = pq[2];
   			   }
   		   }*/


   		   if(distances[i] != 0 && i!=getIndex(radio().id())) {			// Knoten sieht p
   			   for(int j=0; j<neighbourMap.size(); j++) {
   				   if(ndistances[i][j] != 0	&& distances[j] != 0 && getNode(i)!=getNode(j)) {	//q nachbar von p?, knoten sieht q?, p != q?
   	   					   pq[0] = i;				//Setze i=p
   	   					   pq[1] = j;				//Setze j=q
   	   					   pq[2] = 0;				//Score auf 0

   	   					   //Gamma testen, ob innerhalb (-1,1)
   	   					   float dip = distances[i];		//distanz i->p
   	   					   float diq = distances[j];		//distanz i->q
   	   					   float dpq = ndistances[i][j];	//distanz p->q
   	   					   double gammaValue = ((diq*diq) + (dip*dip) - (dpq*dpq)) / (2*diq*dip);

   	   					   if(-1 <= gammaValue && gammaValue <= 1) {
   	   						   //jetzt prüfen, wieviele Nachbarn p und q gemein haben
   	   						   for(int k=0;k<neighbourMap.size();k++) {
   	   						  	if(ndistances[pq[0]][k] != 0 	//p sieht k
   	   						  			&& ndistances[pq[1]][k] != 0  	//q sieht k
   	   						  			&& distances[k] != 0				//knoten sieht k auch
   	   						  			&& pq[0] != k					//p != k
   	   						  			&& pq[1] != k					//q != k
   	   						  			&& getIndex(radio().id()) != k)			//k ist nicht der Knoten selbst
   									{
   										int scoreTemp = pq[2];
   										scoreTemp++;
   										pq[2] = scoreTemp;
   								} //if
   	   						   } //for k
   	   					   } //if
   	   				   } //if
   	   			   } // for j
   			   } //if
   	   } // for i

   	  if(pqBest[2] > 0) {
   	  p_index= pqBest[0];
   	  q_index= pqBest[1];
   	   debug().debug( "p und q gesetzt bei Knoten %i \n", radio().id());

   	   if(coordinates[p_index][1] == 0 && coordinates[q_index][0] == 0 && coordinates[q_index][1] == 0) {
   	   		 findCoordinatesPQ();
   	   		 debug().debug("aufruf findCoordinatesPQ bei Knoten %i \n",radio().id());
   	   }

   	  }

      }	//Funktion

      template<typename OsModel_P,
                      typename Radio_P, typename Distance_P,
                      typename Debug_P>
       void
       Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
       findCoordinatesPQ() {

    	   int self = getIndex(radio().id());
    	   coordinates[self][0] = 0;	//eigenes x: 0
    	   coordinates[self][1] = 0;	//eigenes y: 0

    	   coordinates[p_index][0] = distances[p_index];	//p: x entspricht abstand
    	   coordinates[p_index][1] = 0;				//p: auf x-achse

    	   //q-koordinaten
    	   double gamma = 0.0;
    	   float dip = distances[p_index];		//distanz i->p
    	   float diq = distances[q_index];		//distanz i->q
    	   float dpq = ndistances[p_index][q_index];	//distanz p->q
    	   double gammaValue = ((diq*diq) + (dip*dip) - (dpq*dpq)) / (2*diq*dip);
    	   gamma = acos(gammaValue);
    	   coordinates[q_index][0] = diq * (cos(gamma));
    	   coordinates[q_index][1] = diq * (sin(gamma));

    	   //für weitere knoten j:
    	   double alpha = 0.0;
    	   double beta = 0.0;

    	   for(int j=0;j<NODECOUNT;j++) {
    		   if(ndistances[p_index][j] != 0		//p hat j als nachbarn
    		   && ndistances[q_index][j] != 0		//q auch
    		   && distances[j] != 0			//knoten sieht j auch
    		   && j != self					//j ist man auch nicht selber
    		   && j != p_index					//j ist auch nicht p
    		   && j != q_index) 					//und auch nicht q
    		   {
    			   float dij = distances[j];		//distanz i->j
    			   float dpj = ndistances[p_index][j];	//distanz p->j
    			   float dqj = ndistances[q_index][j];	//distanz q->j
    			   double alphaValue = ((dij*dij) + (dip*dip) - (dpj*dpj)) / (2*dij*dip);
    			   double betaValue = ((diq*diq) + (dij*dij) - (dqj*dqj)) / (2*diq*dij);
    			   if(-1 <= betaValue && betaValue <= 1 && -1 <= alphaValue && alphaValue <= 1) {
    				   alpha = acos(alphaValue);
    				   beta = acos(betaValue);
    				   coordinates[j][0] = dij * (cos(alpha));	//x-koordinate von knoten j
    				   //betrag der y-koordinate von j ...
    				   coordinates[j][1] = dij * (sin(alpha));
    				   //über oder unter x-achse? Toleranzbereich:
    				   double diffGammaAlpha = fabs(alpha-gamma);
    				  double tolerance = 0.6; //TODO: Toleranzwert
    				   if( (diffGammaAlpha-tolerance) <= beta && beta <= (diffGammaAlpha+tolerance) ) {}  	//beta im tol-bereich von |alpha-gamma| -> y bleibt positiv
    					   else coordinates[j][1] = -(coordinates[j][1]);									//sonst: y wird negativ
    			   }
    		   }
    	   }

       } //Funktion



   /**
    * Iteriert über alle vorhandenen Knotenkoordinaten und versucht, kollisionsfreie Kreise zu finden
    * Zur Bewertung der Kreise wird testCircle() aufgerufen.
    * Ein koillisionsfreier Kreis wird in den Vektor foundCircles eingefügt.
    * Anschließend wird eine Nachricht an die beiden beteiligten Knoten versandt, die den Kreis ebenfalls testen werden.
    */
   template<typename OsModel_P,
                 typename Radio_P, typename Distance_P,
                 typename Debug_P>
   void
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   findCircle() {

#ifdef DEBUG_TRIANGULATION
  			   debug().debug( "findCircle() \n");
#endif


	   for(int i=0;i<NODECOUNT;i++) {									//Distanz-Array durchgehen
		   if(distances[i] != 0) {												//wenn Nachbar i existiert
			   for(int j=0; j<neighbourMap.size(); j++) {						//gehe alle seine Nachbarn durch
				   if( ndistances[i][j] != 0									//wenn i einen Nachbarn j hat..
					&& i != j													//und j nicht i ist
					&& (coordinates[i][0] != 0 || coordinates[i][1] != 0)		//und es zu i Koordinaten gibt
					&& (coordinates[j][0] != 0 || coordinates[j][1] != 0)		//und zu j auch
				   )
				   {
					   int testResult = testCircle(i,j);

					   if(testResult == 1) {
						   circle c;

						   if(getNode(i) < getNode(j)) {
							   c.idFirst = getNode(i);
							   c.idSecond = getNode(j);
						   }
						   else {
							   c.idFirst = getNode(j);
							   c.idSecond = getNode(i);
						   }
						   c.ratingFirst = -1;	//-1 kommt bei der Bewertung nicht vor, gibt nur 0,1,2
						   c.ratingSecond = -1;

						   for(unsigned int a=0; a<NODECOUNT; a++) {
							   if(foundCircles[a].idFirst == min(getNode(i),getNode(j))
							   && foundCircles[a].idSecond == max(getNode(i),getNode(j)) ) {
								   break;
							   }
							   else if(a==NODECOUNT-1) {	//gefundener Kreis noch nicht eingetragen
								   foundCircles.push_back(c);
								    //prüfen lassen, nachricht senden
								    Message msg;
								    msg.set_msg_id(CHECK_CIRCLE);
								    msg.set_node_id(getNode(j));
								   //radio().send(getNode(i),msg.buffer_size(),(uint8_t*)&msg);
								   // debug().debug("checkcircle->%i \n",(int)getNode(i));

								    msg.destination = getNode(i);
								    messages.push_back(msg);
								    millis_t random = rand();
								    random = random % MILLIS;
								    timer().template set_timer<self_type, &self_type::message_timer_elapsed>( 800+random ,this , 0 );

								    msg.set_node_id(getNode(i));

								   // radio().send(getNode(j),msg.buffer_size(),(uint8_t*)&msg);
								   // debug().debug( "checkcircle->%i \n", (int)getNode(j));

								    msg.destination = getNode(j);
								    messages.push_back(msg);
								    random = rand();
								    random = random % MILLIS;
								    timer().template set_timer<self_type, &self_type::message_timer_elapsed>( random ,this , 0 );
							   }
						   }
					   }
				   }
			   }
		   }
	   }

   }


   /**
    * Prüft, ob ein Dreieck ein Delaunay-Dreieck ist
    * 0, wenn Koordinaten nicht ausreichen
    * 1, wenn kein anderer Knoten innerhalb des Kreises
    * 2, wenn ein Knoten innerhalb ist
    */
   template<typename OsModel_P,
               typename Radio_P, typename Distance_P,
               typename Debug_P>
   int
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   testCircle(int B, int C) {
	   //1 für: Kreis passt, 2 für: Kreis passt nicht, 0 für: Punkte keine Nachbarn

	   int self = getIndex(radio().id());
	   float xA = coordinates[self][0];				//0
	   float yA = coordinates[self][1];				//0
	   float xB = 0;
	   float yB = 0;
	   float xC = 0;
	   float yC = 0;
	   float x1 = 0;
	   float y1 = 0;
	   float x2 = 0;
	   float y2 = 0;
	   float x3 = 0;
	   float y3 = 0;


	   if(coordinates[B][0] != 0 || coordinates[B][1] != 0) {	//Prüfen ob Koordinaten vorhanden
		   xB = coordinates[B][0];
		   yB = coordinates[B][1];
	   }
	   else {
		   return 0;
	   }

	   if(coordinates[C][0] != 0 || coordinates[C][1] != 0) {	//Prüfen ob Koordinaten vorhanden
		   xC = coordinates[C][0];
		   yC = coordinates[C][1];
	   }
	   else {
	   	   return 0;
	   }


	   //Schnitt 2er Mittelsenkrechten
	   int nullCount = 0;
	   float yAB = yA-yB; if(yAB == 0) nullCount++;
	   float yAC = yA-yC; if(yAC == 0) nullCount++;
	   float yBC = yB-yC; if(yBC == 0) nullCount++;


	   if(nullCount >1) {
		   return 0;
#ifdef DEBUG_TRIANGULATION
		   debug().debug( "zu oft null \n");
#endif
		   }	//keine berechnung möglich weil nenner 2 mal 0 - rückgabe 0 oder 2 ?
	   if(nullCount == 0 || yAC==0) {x1 = xA; y1 = yA; x2 = xB; y2 = yB; x3 = xC; y3= yC;} 	//AB & BC
	   if(yBC==0) {x1 = xC; y1 = yC; x2 = xA; y2 = yA; x3 = xB; y3= yB;} 	//AC + AB <=> CA & AB
	   if(yAB==0) {x1 = xA; y1 = yA; x2 = xC; y2 = yC; x3 = xB; y3= yB;} 	//AC + BC <=> AC & CB


	   //Werte für Mittelsenkrechte 1
	   //y = -(xA-xB)/(yA-yB) * x + xA^2 - xB^2 + yA^2 - yB^2 / 2(yA-yB)
	   double m1first = (x1-x2) / (y1-y2);
	   if((y1-y2) == 0){
#ifdef DEBUG_TRIANGULATION
		   debug().debug( "y1-y2 ist 0; y1 ist %i und y2 ist %i \n",y1,y2);
#endif
	   }
	   m1first = (-1) * m1first;
	   double m1secondZ = (x1*x1) - (x2*x2) + (y1*y1) - (y2*y2);
	   double m1secondN = 2*(y1-y2);
	   double m1second = m1secondZ / m1secondN;

	   //Werte für Mittelsenkrechte 2
	   double m2first = (x2-x3)/(y2-y3);
	   if((y2-y3) == 0) {
#ifdef DEBUG_TRIANGULATION
		   debug().debug( "y2-y3 ist 0; y2 ist %i und y3 ist %i \n",y2,y3);
#endif
	   }
	   m2first = (-1) * m2first;
	   double m2secondZ = (x2*x2) - (x3*x3) + (y2*y2) - (y3*y3);
	   double m2secondN = 2*(y2-y3);
	   double m2second = m2secondZ / m2secondN;

	   //Kreismitte mit x und y
	   double m1fmm2f = m1first - m2first;		//m1-m2
	   if(m1fmm2f == 0) {}
	   double m2smm1s = m2second - m1second;	//n2-n1
	   double x = m2smm1s / m1fmm2f;			//x = ...
	   double y = (m1first * x) + m1second;		//x einsetzen -> y= ...
	   double r2 = sqrt((x2-x)*(x2-x) + (y2-y)*(y2-y));	//Radius des Kreises = Abstand Ursprung zur Mitte


	   //Prüfen, ob Dealaunay-Dreieck
	    for(int k=0;k<NODECOUNT;k++) {												//alle Nachbarn durchgehen
		   if((coordinates[k][0] !=0 || coordinates[k][1] != 0) 					//Koordinaten für k vorhanden
		   && k != self 															//ausschließen aller mitglieder des dreiecks
		   && k != B
		   && k != C) {
			   float testX = coordinates[k][0];
			   float testY = coordinates[k][1];

			   //Abstand zur Kreismitte
			   double distR = sqrt(((testX-x)*(testX-x)) + ((testY-y)*(testY-y)));			//Distanz zur Kreismitte
			   if(distR < r2) {
				   return 2;
			   }
		   }
	   }
	   return 1;
   }

   template<typename OsModel_P,
               typename Radio_P, typename Distance_P,
               typename Debug_P>
   void
   Triangulation<OsModel_P,Radio_P, Distance_P, Debug_P>::
   setRating(node_id_t fromID, node_id_t thirdID, int rating)
   {
	   int self = (int)getIndex(radio().id());

	 //  debug().debug("setRating mit %i,%i,%i \n", (int)fromID,(int)thirdID,(int)rating);
	  // debug().debug( "setRating \n");

	   for(int i=0; i<=(int)foundCircles.size(); i++) {

		  // debug().debug( "for-schleife in setRating \n");
		  // debug().debug( "pruefe, ob %i oder %i zu aufruf passen \n", (int)foundCircles[i].idFirst, (int)foundCircles[i].idSecond);
		  // debug().debug( "undzwar zu %i und %i \n", min(fromID,thirdID),max(fromID,thirdID));


		   if((int)foundCircles[i].idFirst == min(fromID,thirdID)
		   && (int)foundCircles[i].idSecond == max(fromID, thirdID))
		   {

			  // debug().debug( "kreis gefunden in setRating \n");

			   if((int)fromID == (int)foundCircles[i].idFirst) {
				   debug().debug( "set ratingFirst \n");
				   foundCircles[i].ratingFirst = rating;
			   }
			   else if((int)fromID == (int)foundCircles[i].idSecond) {
				   debug().debug( "set ratingSecond \n");
				   foundCircles[i].ratingSecond = rating;
			   }

		   if(foundCircles[i].ratingFirst == 1
		   && foundCircles[i].ratingSecond == 1) {
			   //in globale Triangulierung eintragen
			  debug().debug("triangleglobal aus setrating: %i %i %i",getNode(self),foundCircles[i].idFirst, foundCircles[i].idSecond);
			   //setTriangleGlobal((int)getNode(self), (int)foundCircles[i].idFirst, (int)foundCircles[i].idSecond, 1);
			   setTriangleGlobal(getNode(self),foundCircles[i].idFirst, foundCircles[i].idSecond, 1);
			   break;
		   }
		   if((foundCircles[i].ratingFirst == 1 && foundCircles[i].ratingSecond == 0)		// einer stimmt zu, der andere sieht zuwenig
		   || (foundCircles[i].ratingFirst == 0 && foundCircles[i].ratingSecond == 1))		// und umgedreht
		   {
			   debug().debug("1 1 0 \n");
			   setTriangleGlobal((int)getNode(self), (int)foundCircles[i].idFirst, (int)foundCircles[i].idSecond, 2);
			   break;
		   }
	   }
   }
   }

   template<typename OsModel_P,
                  typename Radio_P, typename Distance_P,
                  typename Debug_P>
   void
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   setTriangleGlobal(node_id_t a, node_id_t b, node_id_t c, int trust) {

	   //! wichtig: a = eigene ID, b = foundCircles.idFirst, c = foundCircles.idSecond

	   //debug().debug("triangleglobal mit %i, %i, %i \n", a,b,c);

	   triangle t;
	   t.trust = trust;

	   if(a<b) {
		   t.idFirst = a;
		   t.idSecond = b;
		   t.idThird = c;
	   }
	   else if(b < a && a < c) {
		   t.idFirst = b;
		   t.idSecond = a;
		   t.idThird = c;
	   }
	   else {
		   t.idFirst = b;
		   t.idSecond = c;
		   t.idThird = a;
	   }


	   //hinzufügen des dreiecks, falls noch nicht im vector enthalten
	   //weitergeben der meldung an die nachbarn
	   for(int i=0; i<VECTORSIZE; i++) {
		   if(triangles[i].idFirst == a && triangles[i].idSecond == b && triangles[i].idThird == c) {	//Dreieck enthalten?
			   if(triangles[i].prop != true) {															//noch nicht propagiert?
				   Message message;																		//versenden ...
				   message.set_msg_id(TRIANGLE_PROP);
				   message.set_node_id(t.idFirst);
				   message.set_seq_nr(t.idSecond);
				   message.set_trust(t.trust);
				   message.set_id(t.idThird);
			   	  // radio().send( radio().BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message);

				   message.destination=radio().BROADCAST_ADDRESS;
			   	   messages.push_back(message);
			   	   millis_t random = rand();
			   	   random = random % MILLIS;
			   	   timer().template set_timer<self_type, &self_type::message_timer_elapsed>( random ,this , 0 );

			   	   triangles[i].prop = true;															//als propagiert kennzeichnen
			   }
			   break;
		   }
		   else if(i==VECTORSIZE-1) {																	//Dreieck noch nicht enthalten
			   t.prop = true;
			   triangles.push_back(t);																	//einfügen und propagieren
			   Message message;
			   message.set_msg_id(TRIANGLE_PROP);
			   message.set_node_id(t.idFirst);
			   message.set_seq_nr(t.idSecond);
			   message.set_trust(t.trust);
			   message.set_id(t.idThird);
			  // radio().send( radio().BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message);

			   message.destination = radio().BROADCAST_ADDRESS;
			   messages.push_back(message);
			   millis_t random = rand();
			   random = random % MILLIS;
			   timer().template set_timer<self_type, &self_type::message_timer_elapsed>( random ,this , 0 );


		   }
	   }
   }

   template<typename OsModel_P,
   typename Radio_P, typename Distance_P,
   typename Debug_P>
   bool
   Triangulation<OsModel_P,Radio_P, Distance_P, Debug_P>::
   nodeInTriangles() {
	   for(unsigned int i=0; i<triangles.size();i++) {
		   if(radio().id() == triangles[i].idFirst || radio().id() == triangles[i].idSecond || radio().id() == triangles[i].idThird) {
			   return true;
		   }
	   }
	   return false;
   }

   template<typename OsModel_P,
   typename Radio_P, typename Distance_P,
   typename Debug_P>
   int
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   bestCircleIndex(node_id_t demandedID) {
	   if(demandedID == radio().id()) {
		   int bestTrust = 20;
		   int arrayIndex = -1;
		   for(unsigned int j=0; j < foundCircles.size(); j++) {
			   int t = lookUpTrust(foundCircles[j].ratingFirst, foundCircles[j].ratingSecond);
			   if(t < bestTrust) {
				   arrayIndex = j;
				   bestTrust = t;
			   }
		   }
		   return arrayIndex;
	   }
	   else {
		   int bestTrust = 20;
		   int arrayIndex = -1;
		   for(unsigned int i=0; i<foundCircles.size();i++) {											//alle kreise durchgehen
			   if(demandedID == foundCircles[i].idFirst || demandedID == foundCircles[i].idSecond) {	//einer der knoten ist der gesuchte
				  int t = lookUpTrust(foundCircles[i].ratingFirst, foundCircles[i].ratingSecond);
				  if(t < bestTrust) {
					  arrayIndex = i;
					  bestTrust = t;
				  }
			   }
		   }
		   return arrayIndex;
	   }
   }

   template<typename OsModel_P,
   typename Radio_P, typename Distance_P,
   typename Debug_P>
   int
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   lookUpTrust(int ratingFirst, int ratingSecond) {

	   //1. 1-1-2 = 3 = gelb
	   //2. 1-0-0 = 4 = orange
	   //3. 1-0-2 = 5 = rot
	   //4. 1-2-2 = 5 = rot

	   if((ratingFirst == 1 && ratingSecond == 2) || (ratingFirst == 2 && ratingSecond == 1)) {
		   return 3;
	   }
	   if(ratingFirst == 0 && ratingSecond == 0) {
		   return 4;
	   }
	   if((ratingFirst == 0 && ratingSecond == 2) || (ratingFirst == 2 && ratingSecond == 0)) {
		   return 5;
	   }
	   if(ratingFirst == 2 && ratingSecond == 2) {
		   return 5;
	   }
	   return 5;
   }

   template<typename OsModel_P,
            typename Radio_P,
            typename Distance_P,
            typename Debug_P>
   void
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   send()
   {
 	  Message msg;


	  // if(!send_called) {

		   send_called = true;

		   if(send_timer_iteration != false) {
			   msg.set_msg_id(DISTANCE_CHECK);
			   radio().send( radio().BROADCAST_ADDRESS, msg.buffer_size(), (uint8_t*)&msg);
		   }


#ifdef DEBUG_TRIANGULATION
		//   debug().debug( "aufruf send \n");
#endif

		   //timer aufrufen - wenn fertig, p und q suchen - bedingung: distanzen vollständig
			 if(coordinates_timer_started == false) {
				 timer().template set_timer<self_type, &self_type::coordinates_timer_elapsed>( waitingTime1, this, 0 );
				 coordinates_timer_started = true;
			 }
	  // }

   }



   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P, typename Distance_P,
            typename Debug_P>
   void
   Triangulation<OsModel_P, Radio_P, Distance_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {

      if ( from == radio().id() )
         return;

#ifdef DEBUG_TRIANGULATION
     debug().debug("msg<-%i \n",(int)from);
#endif

      double distance = dist_est_.distance(from);
      //double distance = 3;

      Message* msgrec = (Message*) data;
      message_id_t msgID = msgrec->msg_id();

      //Distanz-Anfrage; sendet die Distanz an den anfragenden Knoten zurück
      if(msgID == DISTANCE_CHECK && send_timer_iteration != false) {

#ifdef DEBUG_TRIANGULATION
    	 debug().debug( "distance: %i \n",(int)distance);
#endif

    	  //Aufruf der Send-Methode, um den Knoten vor dem Timer zu "aktivieren" und die Timer relativ synchron zu starten
    	  if(send_called == false) send();

    	  if(distance>0) {
    	  Message message;
          message.set_msg_id(DISTANCE_RETURN);
          message.set_distance(distance);
          radio().send(from,message.buffer_size(),(block_data_t*)&message);
    	  }

        if(distances[getIndex(from)] == 0 && distance > 0) {
        	  distances[getIndex(from)] = (float) distance;	//Eintragen des Abstands
        	//  debug().debug( "distances zu %i war leer, beschrieben eben \n", from);
        }
          else if(distance > 0){
        	  distances[getIndex(from)] = ((float)distance + distances[getIndex(from)]) / 2;
        	  //debug().debug( "distances zu %i war schon beschrieben, neuer wert: %i", from, (int) distances[getIndex(from)]);
        	  //den Mittelwert versenden
        	  Message message;
        	  message.set_msg_id(DISTANCE_PROP);
        	  message.set_distance(distances[getIndex(from)]);
        	  message.set_node_id(from);
        	  radio().send(radio().BROADCAST_ADDRESS,len,(block_data_t*)&message);
          }

      }

      //Antwort auf eine Distanzanfrage, in Array speichern
      else if(msgID == DISTANCE_RETURN && send_timer_iteration != false) {

    	  double d = msgrec->distance();

#ifdef DEBUG_TRIANGULATION
    	//  debug().debug( "node %i erhaelt distanz zurueck: %i ", radio().id(), (int)d);
#endif

    	  //Mittelwert aus eigener Messung (distances[from]) und der Messung d des Nachbarn
    	  if(distances[getIndex(from)] == 0) {
    		  distances[getIndex(from)] = d;
        	 // debug().debug( "distances zu %i war leer, beschrieben eben \n", from);
    		  }
    	  else {
    		  distances[getIndex(from)] = (d + distances[getIndex(from)]) / 2;
        	//  debug().debug( "distances zu %i war schon beschrieben, neuer wert: %i", from, (int) distances[getIndex(from)]);


    	  //den Mittelwert versenden
    	  Message message;
    	  message.set_msg_id(DISTANCE_PROP);
    	  message.set_node_id(from);
    	  message.set_distance(distances[getIndex(from)]);
    	  radio().send(radio().BROADCAST_ADDRESS,len,(block_data_t*)&message);
    	  }
      }


      //Verbreiten der Distanzmessung, in Gesamt-Array speichern
      else if(msgID == DISTANCE_PROP && send_timer_iteration != false) {

    	//  debug().debug( "prop erhalten von %i \n", from);

    	  node_id_t nodeID = msgrec->node_id();
    	  ndistances[getIndex(from)][getIndex(nodeID)]= msgrec->distance();
      }

      //anforderung, einen kreis zu prüfen, mit absender und in node_id enthaltenen Knoten
      else if(msgID == CHECK_CIRCLE) {

    	  Message message;

    	  node_id_t nodeID = msgrec->node_id();

    	 // debug().debug( "check_circle von %i, kreis testen mit %i \n", (int)from, (int)nodeID);
    	 // debug().debug( "hole Index von nodeID: %i und von from: %i", (int)getIndex(nodeID), (int)getIndex(from));

    	  int testResult = testCircle(getIndex(nodeID),getIndex(from));

    	  message.set_node_id(nodeID);
    	  message.set_msg_id(CHECK_CIRCLE_RETURN);
    	  message.set_result(testResult);

    	  message.destination = from;
    	  messages.push_back(message);
    	  millis_t random = rand();
    	  random = random % MILLIS;
    	  timer().template set_timer<self_type, &self_type::message_timer_elapsed>( random ,this , 0 );

    	 // radio().send( from,message.buffer_size(), (block_data_t*)&message);
    	 // debug().debug( "result->%i \n", (int)from);
      }

      else if(msgID == CHECK_CIRCLE_RETURN) {

    	  int result = msgrec->result();
    	  node_id_t nodeID = msgrec->node_id();
      	  debug().debug( "received return<-%i", (int)from);
    	  setRating(from, nodeID, result);
      }

      else if(msgID == TRIANGLE_PROP) {
    	  node_id_t a = msgrec->node_id();
    	  node_id_t b = msgrec->seq_nr();
    	//  int c = atoi((char*)msgrec->payload());
    	  node_id_t c = msgrec->id();
    	  int t = msgrec->trust();
    	  debug().debug("triangle_prop: %i %i %i \n",a,b,c);
    	  setTriangleGlobal(a,b,c,t);
      }

      else if(msgID == ASKFORCIRCLE) {
    	  int circleIndex = bestCircleIndex(from);
    	  if (circleIndex>-1) {
    		  Message message;
    		  message.set_msg_id(RETURN_CIRCLE);
    		  if(from != foundCircles[circleIndex].idFirst)
    			   message.set_node_id(foundCircles[circleIndex].idFirst);
    		  else message.set_node_id(foundCircles[circleIndex].idSecond);
    		  message.set_trust(lookUpTrust(foundCircles[circleIndex].ratingFirst,foundCircles[circleIndex].ratingSecond));
    		 //radio().send(from,message.buffer_size(),(block_data_t*)&message);

    		  messages.push_back(message);
    		  millis_t random = rand();
    		  random = random % MILLIS;
    		  timer().template set_timer<self_type, &self_type::message_timer_elapsed>( random ,this , 0 );
    	  }
      }

      else if(msgID == RETURN_CIRCLE) {

    	  //falls noch keinen kreisvorschlag erhalten oder ein neuer kreisvorschlag ist besser als der alte ...
    	  if(demandedCircleIDs[2] == -1 || demandedCircleIDs[2] > msgrec->trust()) {
    		 if(from < msgrec->node_id()) {
    			 demandedCircleIDs[0] = from;
    			 demandedCircleIDs[1] = msgrec->node_id();
    		 }
    		 else {
    			 demandedCircleIDs[0] = msgrec->node_id();
    			 demandedCircleIDs[1] = from;
    		 }
    		 demandedCircleIDs[2] = msgrec->trust();
    	  }

      }

   }

}
#endif

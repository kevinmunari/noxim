/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2018 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the Network-on-Chip
 */

#include "NoC.h"

#include "GraphViz.h"

using namespace std;

void NoC::buildMesh()
{
    
    if (GlobalParams::use_graphviz == true) {
    	// Set up a Graphviz context
    	graphviz_context = gvContext();
    	
    	// Create the graph
    	char graph_name[16] = "network";
    	graph = agopen(graph_name, Agdirected, 0);
    	
    	// Set Graph attribute
    	sprintf(attribute_name, "concentrate");
        if (GlobalParams::detailed == true) {
            sprintf(attribute_value, "false");
        } else {
            sprintf(attribute_value, "true");
        }
    	sprintf(attribute_default_value, "false");
    	agsafeset(graph, attribute_name, attribute_value, attribute_default_value);
    }

    token_ring = new TokenRing("tokenring");
    token_ring->clock(clock);
    token_ring->reset(reset);


    char channel_name[16];
    for (map<int, ChannelConfig>::iterator it = GlobalParams::channel_configuration.begin();
            it != GlobalParams::channel_configuration.end();
            ++it)
    {
        int channel_id = it->first;
        sprintf(channel_name, "Channel_%d", channel_id);
        channel[channel_id] = new Channel(channel_name, channel_id);
        
        if (GlobalParams::use_graphviz == true) {
            if (GlobalParams::use_winoc == true) {
        	// Create a Channel node
        	channel_node[channel_id] = agnode(graph, channel_name, 1);
        	// Set Channel node attributes
        	sprintf(attribute_name, "color");
        	sprintf(attribute_value, "%s", GlobalParams::channel_node_color.c_str());
        	sprintf(attribute_default_value, "black");
        	agsafeset(channel_node[channel_id], attribute_name, attribute_value, attribute_default_value);
        	sprintf(attribute_name, "regular");
        	if (GlobalParams::channel_node_regular == true) {
            		sprintf(attribute_value, "true");
        	} else {
            		sprintf(attribute_value, "false");
        	}
        	sprintf(attribute_default_value, "false");
        	agsafeset(channel_node[channel_id], attribute_name, attribute_value, attribute_default_value);
        	sprintf(attribute_name, "shape");
        	sprintf(attribute_value, "%s", GlobalParams::channel_node_shape.c_str());
        	sprintf(attribute_default_value, "ellipse");
        	agsafeset(channel_node[channel_id], attribute_name, attribute_value, attribute_default_value);
            }
        }
    }

    char hub_name[16];
    for (map<int, HubConfig>::iterator it = GlobalParams::hub_configuration.begin();
            it != GlobalParams::hub_configuration.end();
            ++it)
    {
        int hub_id = it->first;
	//LOG << " hub id " <<  hub_id;
        HubConfig hub_config = it->second;

        sprintf(hub_name, "Hub_%d", hub_id);
        hub[hub_id] = new Hub(hub_name, hub_id,token_ring);
        hub[hub_id]->clock(clock);
        hub[hub_id]->reset(reset);
        
        if (GlobalParams::use_graphviz == true) {
            if (GlobalParams::use_winoc == true) {
        	// Create a Hub node
        	hub_node[hub_id] = agnode(graph, hub_name, 1);
        	// Set Hub node attributes
        	sprintf(attribute_name, "color");
        	sprintf(attribute_value, "%s", GlobalParams::hub_node_color.c_str());
        	sprintf(attribute_default_value, "black");
        	agsafeset(hub_node[hub_id], attribute_name, attribute_value, attribute_default_value);
        	sprintf(attribute_name, "regular");
        	if (GlobalParams::hub_node_regular == true) {
            		sprintf(attribute_value, "true");
        	} else {
            		sprintf(attribute_value, "false");
        	}
        	sprintf(attribute_default_value, "false");
        	agsafeset(hub_node[hub_id], attribute_name, attribute_value, attribute_default_value);
        	sprintf(attribute_name, "shape");
        	sprintf(attribute_value, "%s", GlobalParams::hub_node_shape.c_str());
        	sprintf(attribute_default_value, "ellipse");
        	agsafeset(hub_node[hub_id], attribute_name, attribute_value, attribute_default_value);
            }
        }

        // Determine, from configuration file, which Hub is connected to which Tile
        for(vector<int>::iterator iit = hub_config.attachedNodes.begin(); 
                iit != hub_config.attachedNodes.end(); 
                ++iit) 
        {
            GlobalParams::hub_for_tile[*iit] = hub_id;
        }


        // Determine, from configuration file, which Hub is connected to which Channel
        for(vector<int>::iterator iit = hub_config.txChannels.begin(); 
                iit != hub_config.txChannels.end(); 
                ++iit) 
        {
            int channel_id = *iit;
            //LOG << "Binding " << hub[hub_id]->name() << " to txChannel " << channel_id << endl;
            hub[hub_id]->init[channel_id]->socket.bind(channel[channel_id]->targ_socket);
            //LOG << "Binding " << hub[hub_id]->name() << " to txChannel " << channel_id << endl;
            hub[hub_id]->setFlitTransmissionCycles(channel[channel_id]->getFlitTransmissionCycles(),channel_id);
            
            if (GlobalParams::use_graphviz == true) {
                if (GlobalParams::use_winoc == true) {
            		// Create a Hub->Channel connection
            		Agedge_t *edge = agedge(graph, hub_node[hub_id], channel_node[channel_id], 0, 1);
            		// Set Hub->Channel connection attributes
            		sprintf(attribute_name, "color");
            		sprintf(attribute_value, "%s", GlobalParams::hub_chn_edge_color.c_str());
            		sprintf(attribute_default_value, "black");
            		agsafeset(edge, attribute_name, attribute_value, attribute_default_value);
            		sprintf(attribute_name, "style");
            		sprintf(attribute_value, "%s", GlobalParams::hub_chn_edge_style.c_str());
            		sprintf(attribute_default_value, "");
            		agsafeset(edge, attribute_name, attribute_value, attribute_default_value);
            		sprintf(attribute_name, "tooltip");
                    	if (GlobalParams::detailed == true) {
                        	sprintf(attribute_value, "Hub_%d-->Channel_%d", hub_id, channel_id);
                    	} else {
                        	sprintf(attribute_value, "Hub_%d--Channel_%d", hub_id, channel_id);
                    	}
            		sprintf(attribute_default_value, "");
            		agsafeset(edge, attribute_name, attribute_value, attribute_default_value);
                }
            }
        }

        for(vector<int>::iterator iit = hub_config.rxChannels.begin(); 
                iit != hub_config.rxChannels.end(); 
                ++iit) 
        {
            int channel_id = *iit;
            //LOG << "Binding " << hub[hub_id]->name() << " to rxChannel " << channel_id << endl;
            channel[channel_id]->init_socket.bind(hub[hub_id]->target[channel_id]->socket);
            channel[channel_id]->addHub(hub[hub_id]);
            
            if (GlobalParams::use_graphviz == true) {
                if (GlobalParams::use_winoc == true) {
            		// Create a Hub<-Channel connection
            		Agedge_t *edge = agedge(graph, channel_node[channel_id], hub_node[hub_id], 0, 1);
            		// Set Hub<-Channel connection attributes
            		sprintf(attribute_name, "color");
            		sprintf(attribute_value, "%s", GlobalParams::hub_chn_edge_color.c_str());
            		sprintf(attribute_default_value, "black");
            		agsafeset(edge, attribute_name, attribute_value, attribute_default_value);
            		sprintf(attribute_name, "style");
            		sprintf(attribute_value, "%s", GlobalParams::hub_chn_edge_style.c_str());
            		sprintf(attribute_default_value, "");
            		agsafeset(edge, attribute_name, attribute_value, attribute_default_value);
            		sprintf(attribute_name, "tooltip");
                    	if (GlobalParams::detailed == true) {
                        	sprintf(attribute_value, "Hub_%d<--Channel_%d", hub_id, channel_id);
                    	} else {
                        	sprintf(attribute_value, "Hub_%d--Channel_%d", hub_id, channel_id);
                    	}
            		sprintf(attribute_default_value, "");
            		agsafeset(edge, attribute_name, attribute_value, attribute_default_value);
                }
            }
        }

	// TODO FIX
	// Hub Power model does not currently support different data rates for single hub
	// If multiple channels are connected to an Hub, the data rate
	// of the first channel will be used as default
	
	int no_channels = hub_config.txChannels.size();

	int data_rate_gbs;
	
	if (no_channels > 0) {
	    data_rate_gbs = GlobalParams::channel_configuration[hub_config.txChannels[0]].dataRate;
	}
	else
	    data_rate_gbs = NOT_VALID;

	// TODO: update power model (configureHub to support different tx/tx buffer depth in the power breakdown
	// Currently, an averaged value is used when accounting in Power class methods
	
	hub[hub_id]->power.configureHub(GlobalParams::flit_size,
		                        GlobalParams::hub_configuration[hub_id].toTileBufferSize,
		                        GlobalParams::hub_configuration[hub_id].fromTileBufferSize,
					GlobalParams::flit_size,
					GlobalParams::hub_configuration[hub_id].rxBufferSize,
					GlobalParams::hub_configuration[hub_id].txBufferSize,
					GlobalParams::flit_size,
					data_rate_gbs);
    }


    // Check for routing table availability
    if (GlobalParams::routing_algorithm == ROUTING_TABLE_BASED)
	assert(grtable.load(GlobalParams::routing_table_filename.c_str()));

    // Check for traffic table availability
    if (GlobalParams::traffic_distribution == TRAFFIC_TABLE_BASED)
	assert(gttable.load(GlobalParams::traffic_table_filename.c_str()));

    // Var to track Hub connected ports
    int * hub_connected_ports = (int *) calloc(GlobalParams::hub_configuration.size(), sizeof(int));

    // Initialize signals
    int dimX = GlobalParams::mesh_dim_x + 1;
    int dimY = GlobalParams::mesh_dim_y + 1;

    req = new sc_signal_NSWEH<bool>*[dimX];
    ack = new sc_signal_NSWEH<bool>*[dimX];
    buffer_full_status = new sc_signal_NSWEH<TBufferFullStatus>*[dimX];
    flit = new sc_signal_NSWEH<Flit>*[dimX];

    free_slots = new sc_signal_NSWE<int>*[dimX];
    nop_data = new sc_signal_NSWE<NoP_data>*[dimX];

    for (int i=0; i < dimX; i++) {
        req[i] = new sc_signal_NSWEH<bool>[dimY];
        ack[i] = new sc_signal_NSWEH<bool>[dimY];
	buffer_full_status[i] = new sc_signal_NSWEH<TBufferFullStatus>[dimY];
        flit[i] = new sc_signal_NSWEH<Flit>[dimY];

        free_slots[i] = new sc_signal_NSWE<int>[dimY];
        nop_data[i] = new sc_signal_NSWE<NoP_data>[dimY];
    }

    t = new Tile**[GlobalParams::mesh_dim_x];
    for (int i = 0; i < GlobalParams::mesh_dim_x; i++) {
    	t[i] = new Tile*[GlobalParams::mesh_dim_y];
    }

    if (GlobalParams::use_graphviz == true) {
        // Create Tile nodes
        for (int j = 0; j < GlobalParams::mesh_dim_y; j++) {
            for (int i = 0; i < GlobalParams::mesh_dim_x; i++) {
                char t_name[16];
                Coord t_coord;
                t_coord.x = i;
                t_coord.y = j;
                int t_id = coord2Id(t_coord);
                sprintf(t_name, "Tile_%d", t_id);
                tile_node[t_id] = agnode(graph, t_name, 1);
                // Set Tile node attributes
                sprintf(attribute_name, "color");
                sprintf(attribute_value, "%s", GlobalParams::tile_node_color.c_str());
                sprintf(attribute_default_value, "black");
                agsafeset(tile_node[t_id], attribute_name, attribute_value, attribute_default_value);
                sprintf(attribute_name, "regular");
                if (GlobalParams::tile_node_regular == true) {
                    sprintf(attribute_value, "true");
                } else {
                    sprintf(attribute_value, "false");
                }
                sprintf(attribute_default_value, "false");
                agsafeset(tile_node[t_id], attribute_name, attribute_value, attribute_default_value);
                sprintf(attribute_name, "shape");
                sprintf(attribute_value, "%s", GlobalParams::tile_node_shape.c_str());
                sprintf(attribute_default_value, "ellipse");
                agsafeset(tile_node[t_id], attribute_name, attribute_value, attribute_default_value);
            }
        }
    }


    // Create the mesh as a matrix of tiles
    for (int j = 0; j < GlobalParams::mesh_dim_y; j++) {
	for (int i = 0; i < GlobalParams::mesh_dim_x; i++) {
	    // Create the single Tile with a proper name
	    char tile_name[30];
	    Coord tile_coord;
	    tile_coord.x = i;
	    tile_coord.y = j;
	    int tile_id = coord2Id(tile_coord);
	    sprintf(tile_name, "Tile[%02d][%02d]_(#%d)", i, j, tile_id);
	    t[i][j] = new Tile(tile_name, tile_id);
		

	    // Tell to the router its coordinates
	    t[i][j]->r->configure(j * GlobalParams::mesh_dim_x + i,
				  GlobalParams::stats_warm_up_time,
				  GlobalParams::buffer_depth,
				  grtable);
	    t[i][j]->r->power.configureRouter(GlobalParams::flit_size,
		      			      GlobalParams::buffer_depth,
					      GlobalParams::flit_size,
					      string(GlobalParams::routing_algorithm),
					      "default");
					      


	    // Tell to the PE its coordinates
	    t[i][j]->pe->local_id = j * GlobalParams::mesh_dim_x + i;

    	    // Check for traffic table availability
	    if (GlobalParams::traffic_distribution == TRAFFIC_TABLE_BASED)
	    {
		t[i][j]->pe->traffic_table = &gttable;	// Needed to choose destination
		t[i][j]->pe->never_transmit = (gttable.occurrencesAsSource(t[i][j]->pe->local_id) == 0);
	    }
	    else
		t[i][j]->pe->never_transmit = false;


	    // Map clock and reset
	    t[i][j]->clock(clock);
	    t[i][j]->reset(reset);


            char t_name[16];
            Coord dst_tile_coord;
            int dst_tile_id;
            int edge_id;

	    // Map Rx signals
	    t[i][j]->req_rx[DIRECTION_NORTH] (req[i][j].south);
	    t[i][j]->flit_rx[DIRECTION_NORTH] (flit[i][j].south);
	    t[i][j]->ack_rx[DIRECTION_NORTH] (ack[i][j].north);
	    t[i][j]->buffer_full_status_rx[DIRECTION_NORTH] (buffer_full_status[i][j].north);

            if (GlobalParams::use_graphviz == true) {
                // Create a Tile<-Tile connection in the North direction
                if (GlobalParams::detailed == true) {
                    dst_tile_coord.x = i;
                    dst_tile_coord.y = j-1;
                    dst_tile_id = coord2Id(dst_tile_coord);
                    sprintf(t_name, "Tile_%d", dst_tile_id);
                    if (agnode(graph, t_name, 0) != NULL) {
                        edge_id = tile_id;
                        tile_tile_rx_edge[edge_id].south = agedge(graph, tile_node[dst_tile_id], tile_node[tile_id], 0, 1);
                        // Set Tile<-Tile connection attributes
                        sprintf(attribute_name, "color");
                        sprintf(attribute_value, "%s", GlobalParams::tile_rx_tile_edge_color.c_str());
                        sprintf(attribute_default_value, "black");
                        agsafeset(tile_tile_rx_edge[edge_id].south, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "style");
                        sprintf(attribute_value, "%s", GlobalParams::tile_rx_tile_edge_style.c_str());
                        sprintf(attribute_default_value, "");
                        agsafeset(tile_tile_rx_edge[edge_id].south, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "tooltip");
                        sprintf(attribute_value, "Tile_%d[rx]<--Tile_%d", tile_id, dst_tile_id);
                        sprintf(attribute_default_value, "");
                        agsafeset(tile_tile_rx_edge[edge_id].south, attribute_name, attribute_value, attribute_default_value);
                    }
                }
            }
            

	    t[i][j]->req_rx[DIRECTION_EAST] (req[i + 1][j].west);
	    t[i][j]->flit_rx[DIRECTION_EAST] (flit[i + 1][j].west);
	    t[i][j]->ack_rx[DIRECTION_EAST] (ack[i + 1][j].east);
	    t[i][j]->buffer_full_status_rx[DIRECTION_EAST] (buffer_full_status[i+1][j].east);

            if (GlobalParams::use_graphviz == true) {
                // Create a Tile<-Tile connection in the East direction
                if (GlobalParams::detailed == true) {
                    dst_tile_coord.x = i+1;
                    dst_tile_coord.y = j;
                    if (dst_tile_coord.x < GlobalParams::mesh_dim_x) {
                        dst_tile_id = coord2Id(dst_tile_coord);
                        sprintf(t_name, "Tile_%d", dst_tile_id);
                        if (agnode(graph, t_name, 0) != NULL) {
                            edge_id = dst_tile_id;
                            tile_tile_rx_edge[edge_id].west = agedge(graph, tile_node[dst_tile_id], tile_node[tile_id], 0, 1);
                            // Set Tile<-Tile connection attributes
                            sprintf(attribute_name, "color");
                            sprintf(attribute_value, "%s", GlobalParams::tile_rx_tile_edge_color.c_str());
                            sprintf(attribute_default_value, "black");
                            agsafeset(tile_tile_rx_edge[edge_id].west, attribute_name, attribute_value, attribute_default_value);
                            sprintf(attribute_name, "style");
                            sprintf(attribute_value, "%s", GlobalParams::tile_rx_tile_edge_style.c_str());
                            sprintf(attribute_default_value, "");
                            agsafeset(tile_tile_rx_edge[edge_id].west, attribute_name, attribute_value, attribute_default_value);
                            sprintf(attribute_name, "tooltip");
                            sprintf(attribute_value, "Tile_%d[rx]<--Tile_%d", tile_id, dst_tile_id);
                            sprintf(attribute_default_value, "");
                            agsafeset(tile_tile_rx_edge[edge_id].west, attribute_name, attribute_value, attribute_default_value);
                        }
                    }
                }
            }
            

	    t[i][j]->req_rx[DIRECTION_SOUTH] (req[i][j + 1].north);
	    t[i][j]->flit_rx[DIRECTION_SOUTH] (flit[i][j + 1].north);
	    t[i][j]->ack_rx[DIRECTION_SOUTH] (ack[i][j + 1].south);
	    t[i][j]->buffer_full_status_rx[DIRECTION_SOUTH] (buffer_full_status[i][j+1].south);

            if (GlobalParams::use_graphviz == true) {            
                // Create a Tile<-Tile connection in the South direction
                if (GlobalParams::detailed == true) {
                    dst_tile_coord.x = i;
                    dst_tile_coord.y = j+1;
                    if (dst_tile_coord.y < GlobalParams::mesh_dim_y) {
                        dst_tile_id = coord2Id(dst_tile_coord);
                        sprintf(t_name, "Tile_%d", dst_tile_id);
                        if (agnode(graph, t_name, 0) != NULL) {
                            edge_id = dst_tile_id;
                            tile_tile_rx_edge[edge_id].north = agedge(graph, tile_node[dst_tile_id], tile_node[tile_id], 0, 1);
                            // Set Tile<-Tile connection attributes
                            sprintf(attribute_name, "color");
                            sprintf(attribute_value, "%s", GlobalParams::tile_rx_tile_edge_color.c_str());
                            sprintf(attribute_default_value, "black");
                            agsafeset(tile_tile_rx_edge[edge_id].north, attribute_name, attribute_value, attribute_default_value);
                            sprintf(attribute_name, "style");
                            sprintf(attribute_value, "%s", GlobalParams::tile_rx_tile_edge_style.c_str());
                            sprintf(attribute_default_value, "");
                            agsafeset(tile_tile_rx_edge[edge_id].north, attribute_name, attribute_value, attribute_default_value);
                            sprintf(attribute_name, "tooltip");
                            sprintf(attribute_value, "Tile_%d[rx]<--Tile_%d", tile_id, dst_tile_id);
                            sprintf(attribute_default_value, "");
                            agsafeset(tile_tile_rx_edge[edge_id].north, attribute_name, attribute_value, attribute_default_value);
                        }
                    }
                }
            }
            

	    t[i][j]->req_rx[DIRECTION_WEST] (req[i][j].east);
	    t[i][j]->flit_rx[DIRECTION_WEST] (flit[i][j].east);
	    t[i][j]->ack_rx[DIRECTION_WEST] (ack[i][j].west);
	    t[i][j]->buffer_full_status_rx[DIRECTION_WEST] (buffer_full_status[i][j].west);

            if (GlobalParams::use_graphviz == true) {
                // Create a Tile<-Tile connection in the West direction
                if (GlobalParams::detailed == true) {
                    dst_tile_coord.x = i-1;
                    dst_tile_coord.y = j;
                    dst_tile_id = coord2Id(dst_tile_coord);
                    sprintf(t_name, "Tile_%d", dst_tile_id);
                    if (agnode(graph, t_name, 0) != NULL) {
                        edge_id = tile_id;
                        tile_tile_rx_edge[edge_id].east = agedge(graph, tile_node[dst_tile_id], tile_node[tile_id], 0, 1);
                        // Set Tile<-Tile connection attributes
                        sprintf(attribute_name, "color");
                        sprintf(attribute_value, "%s", GlobalParams::tile_rx_tile_edge_color.c_str());
                        sprintf(attribute_default_value, "black");
                        agsafeset(tile_tile_rx_edge[edge_id].east, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "style");
                        sprintf(attribute_value, "%s", GlobalParams::tile_rx_tile_edge_style.c_str());
                        sprintf(attribute_default_value, "");
                        agsafeset(tile_tile_rx_edge[edge_id].east, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "tooltip");
                        sprintf(attribute_value, "Tile_%d[rx]<--Tile_%d", tile_id, dst_tile_id);
                        sprintf(attribute_default_value, "");
                        agsafeset(tile_tile_rx_edge[edge_id].east, attribute_name, attribute_value, attribute_default_value);
                    }
                }
            }
            

	    // Map Tx signals
	    t[i][j]->req_tx[DIRECTION_NORTH] (req[i][j].north);
	    t[i][j]->flit_tx[DIRECTION_NORTH] (flit[i][j].north);
	    t[i][j]->ack_tx[DIRECTION_NORTH] (ack[i][j].south);
	    t[i][j]->buffer_full_status_tx[DIRECTION_NORTH] (buffer_full_status[i][j].south);

            if (GlobalParams::use_graphviz == true) {
                // Create a Tile->Tile connection in the North direction
                dst_tile_coord.x = i;
                dst_tile_coord.y = j-1;
                dst_tile_id = coord2Id(dst_tile_coord);
                sprintf(t_name, "Tile_%d", dst_tile_id);
                if (agnode(graph, t_name, 0) != NULL) {
                    edge_id = tile_id;
                    tile_tile_tx_edge[edge_id].north = agedge(graph, tile_node[tile_id], tile_node[dst_tile_id], 0, 1);
                    // Set Tile->Tile connection attributes
                    sprintf(attribute_name, "color");
                    sprintf(attribute_value, "%s", GlobalParams::tile_tx_tile_edge_color.c_str());
                    sprintf(attribute_default_value, "black");
                    agsafeset(tile_tile_tx_edge[edge_id].north, attribute_name, attribute_value, attribute_default_value);
                    sprintf(attribute_name, "style");
                    sprintf(attribute_value, "%s", GlobalParams::tile_tx_tile_edge_style.c_str());
                    sprintf(attribute_default_value, "");
                    agsafeset(tile_tile_tx_edge[edge_id].north, attribute_name, attribute_value, attribute_default_value);
                    sprintf(attribute_name, "tooltip");
                    if (GlobalParams::detailed == true) {
                        sprintf(attribute_value, "Tile_%d[tx]-->Tile_%d", tile_id, dst_tile_id);
                    } else {
                        sprintf(attribute_value, "Tile_%d--Tile_%d", tile_id, dst_tile_id);
                    }
                    sprintf(attribute_default_value, "");
                    agsafeset(tile_tile_tx_edge[edge_id].north, attribute_name, attribute_value, attribute_default_value);
                }
            }
            

	    t[i][j]->req_tx[DIRECTION_EAST] (req[i + 1][j].east);
	    t[i][j]->flit_tx[DIRECTION_EAST] (flit[i + 1][j].east);
	    t[i][j]->ack_tx[DIRECTION_EAST] (ack[i + 1][j].west);
	    t[i][j]->buffer_full_status_tx[DIRECTION_EAST] (buffer_full_status[i + 1][j].west);

            if (GlobalParams::use_graphviz == true) {
                // Create a Tile->Tile connection in the East direction
                dst_tile_coord.x = i+1;
                dst_tile_coord.y = j;
                if (dst_tile_coord.x < GlobalParams::mesh_dim_x) {
                    dst_tile_id = coord2Id(dst_tile_coord);
                    sprintf(t_name, "Tile_%d", dst_tile_id);
                    if (agnode(graph, t_name, 0) != NULL) {
                        edge_id = dst_tile_id;
                        tile_tile_tx_edge[edge_id].east = agedge(graph, tile_node[tile_id], tile_node[dst_tile_id], 0, 1);
                        // Set Tile->Tile connection attributes
                        sprintf(attribute_name, "color");
                        sprintf(attribute_value, "%s", GlobalParams::tile_tx_tile_edge_color.c_str());
                        sprintf(attribute_default_value, "black");
                        agsafeset(tile_tile_tx_edge[edge_id].east, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "style");
                        sprintf(attribute_value, "%s", GlobalParams::tile_tx_tile_edge_style.c_str());
                        sprintf(attribute_default_value, "");
                        agsafeset(tile_tile_tx_edge[edge_id].east, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "tooltip");
                        if (GlobalParams::detailed == true) {
                            sprintf(attribute_value, "Tile_%d[tx]-->Tile_%d", tile_id, dst_tile_id);
                        } else {
                            sprintf(attribute_value, "Tile_%d--Tile_%d", tile_id, dst_tile_id);
                        }
                        sprintf(attribute_default_value, "");
                        agsafeset(tile_tile_tx_edge[edge_id].east, attribute_name, attribute_value, attribute_default_value);
                    }
                }
            }
            

	    t[i][j]->req_tx[DIRECTION_SOUTH] (req[i][j + 1].south);
	    t[i][j]->flit_tx[DIRECTION_SOUTH] (flit[i][j + 1].south);
	    t[i][j]->ack_tx[DIRECTION_SOUTH] (ack[i][j + 1].north);
	    t[i][j]->buffer_full_status_tx[DIRECTION_SOUTH] (buffer_full_status[i][j + 1].north);

            if (GlobalParams::use_graphviz == true) {
                // Create a Tile->Tile connection in the South direction
                dst_tile_coord.x = i;
                dst_tile_coord.y = j+1;
                if (dst_tile_coord.y < GlobalParams::mesh_dim_y) {
                    dst_tile_id = coord2Id(dst_tile_coord);
                    sprintf(t_name, "Tile_%d", dst_tile_id);
                    if (agnode(graph, t_name, 0) != NULL) {
                        edge_id = dst_tile_id;
                        tile_tile_tx_edge[edge_id].south = agedge(graph, tile_node[tile_id], tile_node[dst_tile_id], 0, 1);
                        // Set Tile->Tile connection attributes
                        sprintf(attribute_name, "color");
                        sprintf(attribute_value, "%s", GlobalParams::tile_tx_tile_edge_color.c_str());
                        sprintf(attribute_default_value, "black");
                        agsafeset(tile_tile_tx_edge[edge_id].south, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "style");
                        sprintf(attribute_value, "%s", GlobalParams::tile_tx_tile_edge_style.c_str());
                        sprintf(attribute_default_value, "");
                        agsafeset(tile_tile_tx_edge[edge_id].south, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "tooltip");
                        if (GlobalParams::detailed == true) {
                            sprintf(attribute_value, "Tile_%d[tx]-->Tile_%d", tile_id, dst_tile_id);
                        } else {
                            sprintf(attribute_value, "Tile_%d--Tile_%d", tile_id, dst_tile_id);
                        }
                        sprintf(attribute_default_value, "");
                        agsafeset(tile_tile_tx_edge[edge_id].south, attribute_name, attribute_value, attribute_default_value);
                    }
                }
            }
            

	    t[i][j]->req_tx[DIRECTION_WEST] (req[i][j].west);
	    t[i][j]->flit_tx[DIRECTION_WEST] (flit[i][j].west);
	    t[i][j]->ack_tx[DIRECTION_WEST] (ack[i][j].east);
	    t[i][j]->buffer_full_status_tx[DIRECTION_WEST] (buffer_full_status[i][j].east);

            if (GlobalParams::use_graphviz == true) {
                // Create a Tile->Tile connection in the West direction
                dst_tile_coord.x = i-1;
                dst_tile_coord.y = j;
                dst_tile_id = coord2Id(dst_tile_coord);
                sprintf(t_name, "Tile_%d", dst_tile_id);
                if (agnode(graph, t_name, 0) != NULL) {
                    edge_id = tile_id;
                    tile_tile_tx_edge[edge_id].west = agedge(graph, tile_node[tile_id], tile_node[dst_tile_id], 0, 1);
                    // Set Tile->Tile connection attributes
                    sprintf(attribute_name, "color");
                    sprintf(attribute_value, "%s", GlobalParams::tile_tx_tile_edge_color.c_str());
                    sprintf(attribute_default_value, "black");
                    agsafeset(tile_tile_tx_edge[edge_id].west, attribute_name, attribute_value, attribute_default_value);
                    sprintf(attribute_name, "style");
                    sprintf(attribute_value, "%s", GlobalParams::tile_tx_tile_edge_style.c_str());
                    sprintf(attribute_default_value, "");
                    agsafeset(tile_tile_tx_edge[edge_id].west, attribute_name, attribute_value, attribute_default_value);
                    sprintf(attribute_name, "tooltip");
                    if (GlobalParams::detailed == true) {
                        sprintf(attribute_value, "Tile_%d[tx]-->Tile_%d", tile_id, dst_tile_id);
                    } else {
                        sprintf(attribute_value, "Tile_%d--Tile_%d", tile_id, dst_tile_id);
                    }
                    sprintf(attribute_default_value, "");
                    agsafeset(tile_tile_tx_edge[edge_id].west, attribute_name, attribute_value, attribute_default_value);
                }
            }
            

	    // TODO: check if hub signal is always required
	    // signals/port when tile receives(rx) from hub
	    t[i][j]->hub_req_rx(req[i][j].from_hub);
	    t[i][j]->hub_flit_rx(flit[i][j].from_hub);
	    t[i][j]->hub_ack_rx(ack[i][j].to_hub);
	    t[i][j]->hub_buffer_full_status_rx(buffer_full_status[i][j].to_hub);

	    // signals/port when tile transmits(tx) to hub
	    t[i][j]->hub_req_tx(req[i][j].to_hub); // 7, sc_out
	    t[i][j]->hub_flit_tx(flit[i][j].to_hub);
	    t[i][j]->hub_ack_tx(ack[i][j].from_hub);
	    t[i][j]->hub_buffer_full_status_tx(buffer_full_status[i][j].from_hub);

            // TODO: Review port index. Connect each Hub to all its Channels 
            map<int, int>::iterator it = GlobalParams::hub_for_tile.find(tile_id);
            if (it != GlobalParams::hub_for_tile.end())
            {
                int hub_id = GlobalParams::hub_for_tile[tile_id];

                // The next time that the same HUB is considered, the next
                // port will be connected
                int port = hub_connected_ports[hub_id]++;

                hub[hub_id]->tile2port_mapping[t[i][j]->local_id] = port;

                hub[hub_id]->req_rx[port](req[i][j].to_hub);
                hub[hub_id]->flit_rx[port](flit[i][j].to_hub);
                hub[hub_id]->ack_rx[port](ack[i][j].from_hub);
                hub[hub_id]->buffer_full_status_rx[port](buffer_full_status[i][j].from_hub);

                if (GlobalParams::use_graphviz == true) {
                    if (GlobalParams::use_winoc == true) {
                        // Create a Hub<-Tile connection
                        Agedge_t *edge = agedge(graph, tile_node[tile_id], hub_node[hub_id], 0, 1);
                        // Set Hub<-Tile connection attributes
                        sprintf(attribute_name, "color");
                        sprintf(attribute_value, "%s", GlobalParams::tile_hub_edge_color.c_str());
                        sprintf(attribute_default_value, "black");
                        agsafeset(edge, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "constraint");
                        sprintf(attribute_value, "false");
                        sprintf(attribute_default_value, "true");
                        agsafeset(edge, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "style");
                        sprintf(attribute_value, "%s", GlobalParams::tile_hub_edge_style.c_str());
                        sprintf(attribute_default_value, "");
                        agsafeset(edge, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "tooltip");
                        if (GlobalParams::detailed == true) {
                            sprintf(attribute_value, "Hub_%d<--Tile_%d", hub_id, tile_id);
                        } else {
                            sprintf(attribute_value, "Hub_%d--Tile_%d", hub_id, tile_id);
                        }
                        sprintf(attribute_default_value, "");
                        agsafeset(edge, attribute_name, attribute_value, attribute_default_value);
                    }
                }


                hub[hub_id]->flit_tx[port](flit[i][j].from_hub);
                hub[hub_id]->req_tx[port](req[i][j].from_hub);
                hub[hub_id]->ack_tx[port](ack[i][j].to_hub);
                hub[hub_id]->buffer_full_status_tx[port](buffer_full_status[i][j].to_hub);

                if (GlobalParams::use_graphviz == true) {
                    if (GlobalParams::use_winoc == true) {
                        // Create a Hub->Tile connection
                        Agedge_t *edge2 = agedge(graph, hub_node[hub_id], tile_node[tile_id], 0, 1);
                        // Set Hub->Tile connection attributes
                        sprintf(attribute_name, "color");
                        sprintf(attribute_value, "%s", GlobalParams::tile_hub_edge_color.c_str());
                        sprintf(attribute_default_value, "black");
                        agsafeset(edge2, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "constraint");
                        sprintf(attribute_value, "false");
                        sprintf(attribute_default_value, "true");
                        agsafeset(edge2, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "style");
                        sprintf(attribute_value, "%s", GlobalParams::tile_hub_edge_style.c_str());
                        sprintf(attribute_default_value, "");
                        agsafeset(edge2, attribute_name, attribute_value, attribute_default_value);
                        sprintf(attribute_name, "tooltip");
                        if (GlobalParams::detailed == true) {
                            sprintf(attribute_value, "Hub_%d-->Tile_%d", hub_id, tile_id);
                        } else {
                            sprintf(attribute_value, "Hub_%d--Tile_%d", hub_id, tile_id);
                        }
                        sprintf(attribute_default_value, "");
                        agsafeset(edge2, attribute_name, attribute_value, attribute_default_value);
                    }
                }

            }

            // Map buffer level signals (analogy with req_tx/rx port mapping)
	    t[i][j]->free_slots[DIRECTION_NORTH] (free_slots[i][j].north);
	    t[i][j]->free_slots[DIRECTION_EAST] (free_slots[i + 1][j].east);
	    t[i][j]->free_slots[DIRECTION_SOUTH] (free_slots[i][j + 1].south);
	    t[i][j]->free_slots[DIRECTION_WEST] (free_slots[i][j].west);

	    t[i][j]->free_slots_neighbor[DIRECTION_NORTH] (free_slots[i][j].south);
	    t[i][j]->free_slots_neighbor[DIRECTION_EAST] (free_slots[i + 1][j].west);
	    t[i][j]->free_slots_neighbor[DIRECTION_SOUTH] (free_slots[i][j + 1].north);
	    t[i][j]->free_slots_neighbor[DIRECTION_WEST] (free_slots[i][j].east);

	    // NoP 
	    t[i][j]->NoP_data_out[DIRECTION_NORTH] (nop_data[i][j].north);
	    t[i][j]->NoP_data_out[DIRECTION_EAST] (nop_data[i + 1][j].east);
	    t[i][j]->NoP_data_out[DIRECTION_SOUTH] (nop_data[i][j + 1].south);
	    t[i][j]->NoP_data_out[DIRECTION_WEST] (nop_data[i][j].west);

	    t[i][j]->NoP_data_in[DIRECTION_NORTH] (nop_data[i][j].south);
	    t[i][j]->NoP_data_in[DIRECTION_EAST] (nop_data[i + 1][j].west);
	    t[i][j]->NoP_data_in[DIRECTION_SOUTH] (nop_data[i][j + 1].north);
	    t[i][j]->NoP_data_in[DIRECTION_WEST] (nop_data[i][j].east);

	}
    }

    // dummy NoP_data structure
    NoP_data tmp_NoP;

    tmp_NoP.sender_id = NOT_VALID;

    for (int i = 0; i < DIRECTIONS; i++) {
	tmp_NoP.channel_status_neighbor[i].free_slots = NOT_VALID;
	tmp_NoP.channel_status_neighbor[i].available = false;
    }


    // Clear signals for borderline nodes
    Coord edge_coord;
    int edge_id;
    
    for (int i = 0; i <= GlobalParams::mesh_dim_x; i++) {
	req[i][0].south = 0;
	ack[i][0].north = 0;

    	if (GlobalParams::use_graphviz == true) {
            // Clear connections for borderline nodes
            edge_coord.x = i;
            edge_coord.y = 0;
            if (edge_coord.x < GlobalParams::mesh_dim_x) {
            	edge_id = coord2Id(edge_coord);
            	if (tile_tile_rx_edge[edge_id].south != NULL) {
                    agdeledge(graph, tile_tile_rx_edge[edge_id].south);
            	}
            	if (tile_tile_tx_edge[edge_id].south != NULL) {
                    agdeledge(graph, tile_tile_tx_edge[edge_id].south);
            	}
            }
    	}
    
    
	req[i][GlobalParams::mesh_dim_y].north = 0;
	ack[i][GlobalParams::mesh_dim_y].south = 0;

	free_slots[i][0].south.write(NOT_VALID);
	free_slots[i][GlobalParams::mesh_dim_y].north.write(NOT_VALID);

	nop_data[i][0].south.write(tmp_NoP);
	nop_data[i][GlobalParams::mesh_dim_y].north.write(tmp_NoP);

    }

    for (int j = 0; j <= GlobalParams::mesh_dim_y; j++) {
	req[0][j].east = 0;
	ack[0][j].west = 0;

    	if (GlobalParams::use_graphviz == true) {
            // Clear connections for borderline nodes
            edge_coord.x = 0;
            edge_coord.y = j;
            if (edge_coord.y < GlobalParams::mesh_dim_y) {
            	edge_id = coord2Id(edge_coord);
            	if (tile_tile_rx_edge[edge_id].east != NULL) {
                    agdeledge(graph, tile_tile_rx_edge[edge_id].east);
            	}
            	if (tile_tile_tx_edge[edge_id].east != NULL) {
                    agdeledge(graph, tile_tile_tx_edge[edge_id].east);
            	}
            }
    	}
    
    
	req[GlobalParams::mesh_dim_x][j].west = 0;
	ack[GlobalParams::mesh_dim_x][j].east = 0;

	free_slots[0][j].east.write(NOT_VALID);
	free_slots[GlobalParams::mesh_dim_x][j].west.write(NOT_VALID);

	nop_data[0][j].east.write(tmp_NoP);
	nop_data[GlobalParams::mesh_dim_x][j].west.write(tmp_NoP);

    }

    if (GlobalParams::use_graphviz == true) {    
        cout << "Creating the network graph..." << endl;

    	// Compute a layout using dot layout engine
    	gvLayout(graphviz_context, graph, "dot");
    	
    	// Write the graph on files
    	gvRenderFilename(graphviz_context, graph, "gv", "topology.gv");
    	gvRenderFilename(graphviz_context, graph, "png", "topology.png");
    	gvRenderFilename(graphviz_context, graph, "svg", "topology.svg");
    	
    	// Free layout data
    	gvFreeLayout(graphviz_context, graph);
    	
    	// Free graph structures
    	agclose(graph);
    	
    	// Close output files and free context
    	gvFreeContext(graphviz_context);

        cout << "Done!" << endl;
        cout << endl;
    }
    
}

Tile *NoC::searchNode(const int id) const
{
    for (int i = 0; i < GlobalParams::mesh_dim_x; i++)
	for (int j = 0; j < GlobalParams::mesh_dim_y; j++)
	    if (t[i][j]->r->local_id == id)
		return t[i][j];

    return NULL;
}

void NoC::asciiMonitor()
{ 
    //cout << sc_time_stamp().to_double()/GlobalParams::clock_period_ps << endl;
    system("clear");
    //
    // asciishow proof-of-concept #1 free slots
   
    for (int j = 0; j < GlobalParams::mesh_dim_y; j++)
    {
	for (int s = 0; s<3; s++)
	{
	    for (int i = 0; i < GlobalParams::mesh_dim_x; i++)
	    {
		if (s==0)
		    std::printf("|  %d  ",t[i][j]->r->buffer[s][0].getCurrentFreeSlots());
		else
		    if (s==1)
			std::printf("|%d   %d",t[i][j]->r->buffer[s][0].getCurrentFreeSlots(),t[i][j]->r->buffer[3][0].getCurrentFreeSlots());
		    else
			std::printf("|__%d__",t[i][j]->r->buffer[2][0].getCurrentFreeSlots());
	    }
	    cout << endl;
	}
    }
}


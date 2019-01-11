/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2018 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the global params needed by Noxim
 * to show the network topology
 */

#ifndef __NOXIMGRAPHVIZ_H__
#define __NOXIMGRAPHVIZ_H__

#include <gvc.h>
#include <map>

using namespace std;

GVC_t *gvc;
Agraph_t *graph;
map<int, Agnode_t*> channel_node;
map<int, Agnode_t*> hub_node;
map<int, Agnode_t*> tile_node;

#endif

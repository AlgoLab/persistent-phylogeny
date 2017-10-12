#include "functions.hpp"
#include <fstream>


//=============================================================================
// General functions

// Red-Black Graph

void print_rbgraph(const RBGraph& g) {
  RBVertexIter v, v_end;
  boost::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    std::cout << g[*v].name << ":";
    
    RBOutEdgeIter e, e_end;
    boost::tie(e, e_end) = out_edges(*v, g);
    for (; e != e_end; ++e) {
      std::cout << " -" << (g[*e].color == Color::red ? "r" : "-") << "- "
                << g[target(*e, g)].name
                << ";";
    }
    
    std::cout << std::endl;
  }
}

// File I/O

void read_graph(const std::string filename, RBGraph& g) {
  std::vector<RBVertex> species, characters;
  bool value, first_line = true;
  std::string line;
  std::ifstream file(filename);
  
  size_t idx = 0;
  while (std::getline(file, line)) {
    // for each line in file
    std::istringstream iss(line);
    
    if (first_line) {
      // read rows and columns (species and characters)
      iss >> g[boost::graph_bundle].num_species;
      iss >> g[boost::graph_bundle].num_characters;
      
      species.resize(g[boost::graph_bundle].num_species);
      characters.resize(g[boost::graph_bundle].num_characters);
      
      // insert species in the graph
      for (size_t j = 0; j < g[boost::graph_bundle].num_species; ++j) {
        RBVertex s = add_vertex(g);
        g[s].name = ("s" + std::to_string(j + 1));
        
        species[j] = s;
      }
      
      // insert characters in the graph
      for (size_t j = 0; j < g[boost::graph_bundle].num_characters; ++j) {
        RBVertex c = add_vertex(g);
        g[c].name = ("c" + std::to_string(j + 1));
        g[c].type = Type::character;
        
        characters[j] = c;
      }
      
      first_line = false;
    }
    else {
      // read binary matrix
      while (iss >> value) {
        if (value) {
          // add (black) edge between species[s_idx] and characters[c_idx]
          size_t s_idx = idx / g[boost::graph_bundle].num_characters,
                 c_idx = idx % g[boost::graph_bundle].num_characters;
          
          add_edge(species[s_idx], characters[c_idx], g);
        }
        
        ++idx;
      }
    }
  }
}

// std::tuple<size_t, size_t>
// read_matrix(const std::string filename, std::vector<bool>& m) {
//   size_t num_species, num_characters;
//   bool value, first_line = true;
//   std::string line;
//   std::ifstream file(filename);
  
//   size_t i = 0;
//   while (std::getline(file, line)) {
//     std::istringstream iss(line);
    
//     if (first_line) {
//       // read rows and columns (species and characters)
//       iss >> num_species;
//       iss >> num_characters;
      
//       m.resize(num_species * num_characters);
      
//       first_line = false;
//     }
//     else {
//       // read binary matrix
//       while (iss >> value) {
//         m[i] = value;
//         ++i;
//       }
//     }
//   }
  
//   return std::make_tuple(num_species, num_characters);
// }

// Hasse Diagram

void print_hdgraph(const HDGraph& g) {
  HDVertexIter v, v_end;
  boost::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    std::cout << "[ ";
    
    std::list<std::string>::const_iterator i = g[*v].vertices.begin();
    for (; i != g[*v].vertices.end(); ++i)
      std::cout << *i << " ";
    
    std::cout << "( ";
    
    i = g[*v].characters.begin();
    for (; i != g[*v].characters.end(); ++i)
      std::cout << *i << " ";
    
    std::cout << ") ]:";
    
    HDOutEdgeIter e, e_end;
    boost::tie(e, e_end) = out_edges(*v, g);
    for (; e != e_end; ++e) {
      HDVertex vt = target(*e, g);
      
      std::cout << " -" << g[*e].character;
      
      if (g[*e].state == State::gain)
        std::cout << "+";
      else if (g[*e].state == State::lose)
        std::cout << "-";
      
      std::cout << "-> [ ";
      
      std::list<std::string>::const_iterator i = g[vt].vertices.begin();
      for (; i != g[vt].vertices.end(); ++i)
        std::cout << *i << " ";
      
      std::cout << "( ";
      
      i = g[vt].characters.begin();
      for (; i != g[vt].characters.end(); ++i)
        std::cout << *i << " ";
      
      std::cout << ") ];";
    }
    
    std::cout << std::endl;
  }
}


//=============================================================================
// Algorithm functions

// Red-Black Graph

bool if_singleton::operator()(const RBVertex v, RBGraph& g) const {
  return (out_degree(v, g) == 0);
}

void remove_singletons(RBGraph& g) {
  RBVertexIter v, v_end, next;
  boost::tie(v, v_end) = vertices(g);
  for (next = v; v != v_end; v = next) {
    ++next;
    remove_vertex_if(*v, if_singleton(), g);
  }
}

bool is_free(const RBVertex v, const RBGraph& g) {
  if (g[v].type != Type::character)
    return false;
  
  size_t count_species = 0;
  
  RBOutEdgeIter e, e_end;
  boost::tie(e, e_end) = out_edges(v, g);
  for (; e != e_end; ++e) {
    if (g[*e].color == Color::black || g[target(*e, g)].type != Type::species)
      return false;
    
    count_species++;
  }
  
  if (count_species != g[boost::graph_bundle].num_species)
    return false;
  
  return true;
}

bool is_universal(const RBVertex v, const RBGraph& g) {
  if (g[v].type != Type::character)
    return false;
  
  size_t count_species = 0;
  
  RBOutEdgeIter e, e_end;
  boost::tie(e, e_end) = out_edges(v, g);
  for (; e != e_end; ++e) {
    if (g[*e].color == Color::red || g[target(*e, g)].type != Type::species)
      return false;
    
    count_species++;
  }
  
  if (count_species != g[boost::graph_bundle].num_species)
    return false;
  
  return true;
}

size_t connected_components(const RBGraph& g, RBGraphVector& components) {
  size_t num_comps;
  IndexMap map_index, map_comp;
  AssocMap i_map(map_index), c_map(map_comp);
  
  // fill vertex index map
  RBVertexIter v, v_end;
  size_t idx = 0;
  boost::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v, ++idx) {
    boost::put(i_map, *v, idx);
  }
  
  // get number of components and the components map
  num_comps = boost::connected_components(
    g, c_map, boost::vertex_index_map(i_map)
  );
  
  if (num_comps > 1) {
    // graph is disconnected
    // resize subgraph components
    components.resize(num_comps);
    // vertices: graphVertex => compVertex
    std::map<RBVertex, RBVertex> vertices;
    
    // initialize subgraph components
    for (size_t i = 0; i < num_comps; ++i) {
      components[i] = std::unique_ptr<RBGraph>(new RBGraph);
    }
    
    // add vertices to their respective subgraph
    IndexMap::iterator i = map_comp.begin();
    for (; i != map_comp.end(); ++i) {
      // for each vertex
      RBVertex v = i->first;
      RBVertexSize comp = i->second;
      RBGraph* component = components[comp].get();
      
      vertices[v] = add_vertex(*component);
      (*component)[vertices[v]].name = g[v].name;
      (*component)[vertices[v]].type = g[v].type;
      
      if (g[v].type == Type::species)
        (*component)[boost::graph_bundle].num_species++;
      else
        (*component)[boost::graph_bundle].num_characters++;
    }
    
    // add edges to their respective vertices and subgraph
    i = map_comp.begin();
    for (; i != map_comp.end(); ++i) {
      // for each vertex
      RBVertex v = i->first;
      RBVertex new_v = vertices[v];
      RBVertexSize comp = i->second;
      RBGraph* component = components[comp].get();
      
      // prevent duplicate edges
      // if (g[v].type != Type::species)
      //   continue;
      
      RBOutEdgeIter e, e_end;
      boost::tie(e, e_end) = out_edges(v, g);
      for (; e != e_end; ++e) {
        // for each out edge
        RBVertex new_vt = vertices[target(*e, g)];
        
        RBEdge edge;
        boost::tie(edge, std::ignore) = add_edge(new_v, new_vt, *component);
        (*component)[edge].color = g[*e].color;
      }
    }
    
    #ifdef DEBUG
    std::cout << "Connected components:" << std::endl;
    for (size_t i = 0; i < num_comps; ++i) {
      print_rbgraph(*components[i].get());
      std::cout << std::endl;
    }
    #endif
  }
  #ifdef DEBUG
  else std::cout << "G connected" << std::endl;
  #endif
  
  return num_comps;
}

std::list<RBVertex> maximal_characters(const RBGraph& g) {
  std::list<RBVertex> cm;
  std::map< RBVertex, std::list<RBVertex> > sets;
  int count_incl, count_excl;
  bool keep_char, skip_cycle;
  
  /* How sets is going to be structured:
   * sets[C] => < List of adjacent species to C >
   */
  
  RBVertexIter v, v_end;
  boost::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    if (g[*v].type != Type::character)
      continue;
    // for each character vertex
    
    // build v's set of adjacent species
    RBOutEdgeIter e, e_end;
    boost::tie(e, e_end) = out_edges(*v, g);
    for (; e != e_end; ++e) {
      RBVertex vt = target(*e, g);
      
      // if v is active or connected to random nodes ignore it
      if (g[*e].color == Color::red || g[vt].type != Type::species)
        break;
      
      sets[*v].push_back(vt);
    }
    
    if (cm.empty()) {
      // cm being empty means v can be added without further checks
      cm.push_back(*v);
      continue;
    }
    
    #ifdef DEBUG
    std::cout << g[*v].name << std::endl;
    #endif
    
    // sets[*v] now contains the list of species adjacent to v
    
    skip_cycle = false;
    
    // check if sets[*v] is subset of the species adjacent to cmv
    RBVertexIter cmv = cm.begin(), cmv_end = cm.end();
    while (cmv != cmv_end) {
      // for each species in cm
      if (skip_cycle)
        break;
      
      #ifdef DEBUG
      std::cout << "curr Cm: " << g[*cmv].name << " = { ";
      RBVertexIter kk = sets[*cmv].begin(), kk_end = sets[*cmv].end();
      for (; kk != kk_end; ++kk) std::cout << g[*kk].name << " ";
      std::cout << "}:" << std::endl;
      #endif
      
      count_incl = 0; count_excl = 0;
      keep_char = false;
      
      RBVertexIter sv = sets[*v].begin(), sv_end = sets[*v].end();
      while (sv != sv_end) {
        // for each species adjacent to v, S(C#)
        #ifdef DEBUG
        std::cout << "S(" << g[*v].name << "): " << g[*sv].name << " -> ";
        #endif
        
        // find sv in the list of cmv's adjacent species
        RBVertexIter in = std::find(sets[*cmv].begin(), sets[*cmv].end(),
                                  *sv);
        
        /* keep count of how many species are included (or not found) in
         * the list of cmv's adjacent species
         */
        if (in != sets[*cmv].end())
          count_incl++;
        else
          count_excl++;
        
        #ifdef DEBUG
        std::cout << count_incl << " " << count_excl;
        #endif
        
        if (std::next(sv) == sv_end) {
          // last iteration on the species in the list has been performed
          if (count_incl == sets[*cmv].size() && count_excl > 0) {
            /* the list of adjacent species to v is a superset of the list of
             * adjacent species to cmv, which means cmv can be replaced
             * by v in the list of maximal characters Cm
             */
            #ifdef DEBUG
            std::cout << " subst" << std::endl;
            #endif
            
            cm.push_front(*v);
            cm.remove(*(cmv++));
            skip_cycle = true;
            break;
          }
          else if (count_incl < sets[*cmv].size() && count_excl > 0) {
            /* the list of adjacent species to v is neither a superset nor a
             * subset of the list of adjacent species to cmv, which means
             * v may be a new maximal character
             */
            #ifdef DEBUG
            std::cout << " add? not subset?";
            #endif
            
            keep_char = true;
          }
          else if (count_incl == sets[*cmv].size()) {
            /* the list of adjacent species to v is the same as the list of
             * adjacent species to cmv, so v can be ignored in the next
             * iterations on the characters in Cm
             */
            #ifdef DEBUG
            std::cout << " ignore, same set?" << std::endl;
            #endif
            
            skip_cycle = true;
            break;
          }
          else if (count_excl == 0) {
            /* the list of adjacent species to v is a subset of the list of
             * adjacent species to cmv, meaning v can be ignored in the
             * next iterations on the characters in Cm
             */
            #ifdef DEBUG
            std::cout << " ignore, subset?" << std::endl;
            #endif
            
            skip_cycle = true;
            break;
          }
          else {
            // how we ended up here nobody knows
            #ifdef DEBUG
            std::cout << " idk?";
            #endif
          }
        }
        
        #ifdef DEBUG
        std::cout << std::endl;
        #endif
        
        ++sv;
      }
      
      if (std::next(cmv) == cmv_end) {
        // last iteration on the characters in the list has been performed
        if (keep_char) {
          /* after all the iterations keep_char is true if the list of adjacent
           * species to v is neither a superset nor a subset of the lists of
           * adjacent species to the characters in Cm, so v is a maximal
           * characters and can be added to the list of maximal characters Cm
           */
          cm.push_front(*v);
          #ifdef DEBUG
          std::cout << "\tadd" << std::endl;
          #endif
        }
      }
      
      #ifdef DEBUG
      std::cout << std::endl;
      #endif
      
      ++cmv;
    }
  }
  
  return cm;
}

// TODO: test which one is faster

std::list<RBVertex> maximal_characters2(const RBGraph& g) {
  std::list<RBVertex> cm;
  std::vector< std::list<RBVertex> > sets(
    g[boost::graph_bundle].num_characters
  );
  std::map< RBVertex, std::list<RBVertex> > v_map;
  int count_incl, count_excl;
  bool keep_char, skip_cycle;
  
  /* How sets is going to be structured:
   * sets[index] => < C, List of adjacent species to C >
   *
   * sets is used to sort the lists by number of elements, this is why we store
   * the list of adjacent species to C. While we store C to be able to access
   * v_map[C] in costant time
   *
   * How v_map is going to be structured:
   * v_map[C] => < List of adjacent species to C >
   */
  
  size_t idx = 0;
  RBVertexIter v, v_end;
  boost::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    if (g[*v].type != Type::character)
      continue;
    // for each character vertex
    
    sets[idx].push_back(*v);
    
    // build v's set of adjacent species
    RBOutEdgeIter e, e_end;
    boost::tie(e, e_end) = out_edges(*v, g);
    for (; e != e_end; ++e) {
      RBVertex vt = target(*e, g);
      
      // if v is active or connected to random nodes ignore it
      if (g[*e].color == Color::red || g[vt].type != Type::species)
        break;
      
      sets[idx].push_back(vt);
      v_map[*v].push_back(vt);
    }
    
    ++idx;
  }
  
  // sort sets by size in descending order
  std::sort(sets.begin(), sets.end(), descending_size);
  
  for (size_t i = 0; i < sets.size(); ++i) {
    // for each set of species
    RBVertex v = sets[i].front();
    
    if (sets[i].size() == sets[0].size()) {
      // both sets[i] and sets[0] are maximal
      // or i = 0, which still means sets[i] and sets[0] maximal
      cm.push_back(v);
      continue;
    }
    
    #ifdef DEBUG
    std::cout << g[v].name << std::endl;
    #endif
    
    skip_cycle = false;
    
    // check if sc is subset of the species adjacent to cmv
    RBVertexIter cmv = cm.begin(), cmv_end = cm.end();
    while (cmv != cmv_end) {
      // for each species in cm
      if (skip_cycle)
        break;
      
      #ifdef DEBUG
      std::cout << "curr Cm: " << g[*cmv].name << " = { ";
      RBVertexIter kk = v_map[*cmv].begin(), kk_end = v_map[*cmv].end();
      for (; kk != kk_end; ++kk) std::cout << g[*kk].name << " ";
      std::cout << "}:" << std::endl;
      #endif
      
      count_incl = 0; count_excl = 0;
      keep_char = false;
      
      RBVertexIter sv = v_map[v].begin(), sv_end = v_map[v].end();
      while (sv != sv_end) {
        // for each species adjacent to v, S(C#)
        #ifdef DEBUG
        std::cout << "S(" << g[v].name << "): " << g[*sv].name << " -> ";
        #endif
        
        // find sv in the list of cmv's adjacent species
        RBVertexIter in = std::find(v_map[*cmv].begin(), v_map[*cmv].end(),
                                  *sv);
        
        /* keep count of how many species are included (or not found) in
         * the list of cmv's adjacent species
         */
        if (in != v_map[*cmv].end())
          count_incl++;
        else
          count_excl++;
        
        #ifdef DEBUG
        std::cout << count_incl << " " << count_excl;
        #endif
        
        if (std::next(sv) == sv_end) {
          // last iteration on the species in the list has been performed
          if (count_incl < v_map[*cmv].size() && count_excl > 0) {
            /* the list of adjacent species to v is neither a superset nor a
             * subset of the list of adjacent species to cmv, which means
             * v may be a new maximal character
             */
            #ifdef DEBUG
            std::cout << " add? not subset?";
            #endif
            
            keep_char = true;
          }
          else if (count_incl == v_map[*cmv].size()) {
            /* the list of adjacent species to v is the same as the list of
             * adjacent species to cmv, so v can be ignored in the next
             * iterations on the characters in Cm
             */
            #ifdef DEBUG
            std::cout << " ignore, same set?" << std::endl;
            #endif
            
            skip_cycle = true;
            break;
          }
          else if (count_excl == 0) {
            /* the list of adjacent species to v is a subset of the list of
             * adjacent species to cmv, meaning v can be ignored in the
             * next iterations on the characters in Cm
             */
            #ifdef DEBUG
            std::cout << " ignore, subset?" << std::endl;
            #endif
            
            skip_cycle = true;
            break;
          }
          else {
            // how we ended up here nobody knows
            #ifdef DEBUG
            std::cout << " idk?";
            #endif
          }
        }
        
        #ifdef DEBUG
        std::cout << std::endl;
        #endif
        
        ++sv;
      }
      
      if (std::next(cmv) == cmv_end) {
        // last iteration on the characters in the list has been performed
        if (keep_char) {
          /* after all the iterations keep_char is true if the list of adjacent
           * species to v is neither a superset nor a subset of the lists of
           * adjacent species to the characters in Cm, so v is a maximal
           * characters and can be added to the list of maximal characters Cm
           */
          cm.push_front(v);
          #ifdef DEBUG
          std::cout << "\tadd" << std::endl;
          #endif
        }
      }
      
      #ifdef DEBUG
      std::cout << std::endl;
      #endif
      
      ++cmv;
    }
  }
  
  return cm;
}

bool if_not_maximal::operator()(const RBVertex v, const RBGraph& g) const {
  return (std::find(cm.begin(), cm.end(), v) == cm.end());
}

void maximal_reducible_graph(RBGraph& g, const std::list<RBVertex>& cm) {
  // remove non-maximal characters
  RBVertexIter v, v_end, next;
  boost::tie(v, v_end) = vertices(g);
  for (next = v; v != v_end; v = next) {
    ++next;
    
    if (g[*v].type == Type::character) {
      remove_vertex_if(*v, if_not_maximal(cm), g);
    }
  }
}

void hasse_diagram(const RBGraph& g, HDGraph& hasse) {
  std::vector< std::list<RBVertex> > sets(
    g[boost::graph_bundle].num_species
  );
  std::map< RBVertex, std::list<RBVertex> > v_map;
  
  /* How sets is going to be structured:
   * sets[index] => < S, List of adjacent characters to S >
   *
   * sets is used to sort the lists by number of elements, this is why we store
   * the list of adjacent characters to S. While we store S to be able to
   * access v_map[S] in costant time
   *
   * How v_map is going to be structured:
   * v_map[S] => < List of adjacent characters to S >
   */
  
  size_t idx = 0;
  RBVertexIter v, v_end;
  boost::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    if (g[*v].type != Type::species)
      continue;
    // for each species vertex
    
    #ifdef DEBUG
    std::cout << "C(" << g[*v].name << ") = { ";
    #endif
    
    sets[idx].push_back(*v);
    
    // build v's set of adjacent characters
    RBOutEdgeIter e, e_end;
    boost::tie(e, e_end) = out_edges(*v, g);
    for (; e != e_end; ++e) {
      RBVertex vt = target(*e, g);
      
      #ifdef DEBUG
      std::cout << g[vt].name << " ";
      #endif
      
      sets[idx].push_back(vt);
      v_map[*v].push_back(vt);
    }
    
    #ifdef DEBUG
    std::cout << "}" << std::endl;
    #endif
    
    ++idx;
  }
  
  #ifdef DEBUG
  std::cout << std::endl;
  #endif
  
  // sort sets by size in ascending order
  std::sort(sets.begin(), sets.end(), ascending_size);
  
  for (size_t i = 0; i < sets.size(); ++i) {
    // for each set of characters
    RBVertex v = sets[i].front();
    
    // fill the list of characters names of v
    std::list<std::string> lcv;
    RBVertexIter cv = v_map[v].begin(), cv_end = v_map[v].end();
    for (; cv != cv_end; ++cv)
      lcv.push_back(g[*cv].name);
    
    if (sets[i].size() == sets[0].size()) {
      // first block of sets with the same number of elements
      HDVertex u = add_vertex(hasse);
      hasse[u].vertices.push_back(g[v].name);
      hasse[u].characters = lcv;
      
      continue;
    }
    
    #ifdef DEBUG
    std::cout << g[v].name << " = { ";
    RBVertexIter kk = v_map[v].begin(), kk_end = v_map[v].end();
    for (; kk != kk_end; ++kk) std::cout << g[*kk].name << " ";
    std::cout << "}:" << std::endl;
    #endif
    
    /* new_edges will contain the list of edges that may be added to the
     * Hasse diagram: HDVertex is the source, std::string is the edge label
     */
    std::list< std::pair<HDVertex, std::string> > new_edges;
    
    /* check if there is a vertex with the same characters as v or
     * if v needs to be added to the Hasse diagram
     */
    HDVertexIter hdv, hdv_end;
    boost::tie(hdv, hdv_end) = vertices(hasse);
    while (hdv != hdv_end) {
      // for each vertex in hasse
      #ifdef DEBUG
      std::cout << "hdv: ";
      std::list<std::string>::const_iterator kk = hasse[*hdv].vertices.begin();
      for (; kk != hasse[*hdv].vertices.end(); ++kk) std::cout << *kk << " ";
      std::cout << "= { ";
      kk = hasse[*hdv].characters.begin();
      for (; kk != hasse[*hdv].characters.end(); ++kk) std::cout << *kk << " ";
      std::cout << "} -> ";
      #endif
      
      if (lcv == hasse[*hdv].characters) {
        // v and hdv have the same characters
        #ifdef DEBUG
        std::cout << "mod" << std::endl;
        #endif
        
        // add v (species) to the list of vertices in hdv
        hasse[*hdv].vertices.push_back(g[v].name);
        
        break;
      }
      
      std::list<std::string> lhdv = hasse[*hdv].characters;
      
      if (lhdv.size() < lcv.size()) {
        // hdv has less characters than v
        // Hasse.addE s1 -c1+-> s2
        std::list<std::string>::const_iterator ci, ci_end;
        ci = lcv.begin(); ci_end = lcv.end();
        while (ci != ci_end) {
          // for each character in hdv
          std::list<std::string>::const_iterator in;
          in = std::find(lhdv.begin(), lhdv.end(), *ci);
          
          if (in == lhdv.end()) {
            // character is not present in lhdv
            new_edges.push_back(std::make_pair(*hdv, *ci));
            
            // break;
          }
          
          ++ci;
        }
      }
      
      if (std::next(hdv) == hdv_end) {
        // last iteration on the characters in the list has been performed
        #ifdef DEBUG
        std::cout << "add" << std::endl;
        #endif
        
        // build a vertex for v and add it to the Hasse diagram
        HDVertex u = add_vertex(hasse);
        hasse[u].vertices.push_back(g[v].name);
        hasse[u].characters = lcv;
        
        // build in_edges for the vertex and add them to the Hasse diagram
        std::list< std::pair<HDVertex, std::string> >::iterator ei, ei_end;
        ei = new_edges.begin(), ei_end = new_edges.end();
        for (; ei != ei_end; ++ei) {
          // for each new edge to add to the Hasse diagram
          HDEdge edge;
          boost::tie(edge, std::ignore) = add_edge(ei->first, u, hasse);
          hasse[edge].character = ei->second;
          hasse[edge].state = State::gain;
          
          #ifdef DEBUG
          std::cout << "Hasse.addE ";
          kk = hasse[ei->first].vertices.begin();
          for (; kk != hasse[ei->first].vertices.end(); ++kk)
            std::cout << *kk << " ";
          std::cout << "-" << hasse[edge].character;
          if (hasse[edge].state == State::gain) std::cout << "+";
          else if (hasse[edge].state == State::lose) std::cout << "-";
          std::cout << "-> ";
          kk = hasse[u].vertices.begin();
          for (; kk != hasse[u].vertices.end(); ++kk) std::cout << *kk << " ";
          std::cout << std::endl;
          #endif
        }
        
        break;
      }
      #ifdef DEBUG
      else
        std::cout << "ignore";
      #endif
      
      #ifdef DEBUG
      std::cout << std::endl;
      #endif
      
      ++hdv;
    }
    
    #ifdef DEBUG
    std::cout << std::endl;
    #endif
  }
}


//=============================================================================
// Algorithm main functions

std::list<std::string> reduce(RBGraph& g) {
  std::list<std::string> output;
  
  // cleanup graph from dead vertices
  remove_singletons(g);
  
  if (is_empty(g)) {
    /* if graph is empty
     * return the empty sequence
     */
    #ifdef DEBUG
    std::cout << "G empty"
              << std::endl << std::endl;
    #endif
    
    return output;
  }
  
  #ifdef DEBUG
  std::cout << "G not empty" << std::endl;
  #endif
  
  RBVertexIter v, v_end;
  boost::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    // for each vertex
    if (is_free(*v, g)) {
      /* if v is free
       * realize v-
       * return < v-, reduce(g) >
       */
      #ifdef DEBUG
      std::cout << "G free character " << g[*v].name
                << std::endl << std::endl;
      #endif
      
      // realize(v-, g)
      // ...
      
      output.push_back(g[*v].name + "-");
      output.splice(output.end(), reduce(g));
      
      // return < v-, reduce(g) >
      return output;
    }
  }
  
  #ifdef DEBUG
  std::cout << "G no free characters" << std::endl;
  #endif
  
  boost::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    // for each vertex
    if (is_universal(*v, g)) {
      /* if v is universal
       * realize v+
       * return < v+, reduce(g) >
       */
      #ifdef DEBUG
      std::cout << "G universal character " << g[*v].name
                << std::endl << std::endl;
      #endif
      
      // realize(v+, g)
      // ...
      
      output.push_back(g[*v].name + "+");
      output.splice(output.end(), reduce(g));
      
      // return < v+, reduce(g) >
      return output;
    }
  }
  
  #ifdef DEBUG
  std::cout << "G no universal characters" << std::endl;
  #endif
  
  RBGraphVector components;
  size_t num_comps = connected_components(g, components);
  if (num_comps > 1) {
    /* if graph is not connected
     * build subgraphs (connected components) g1, g2, etc.
     * return < reduce(g1), reduce(g2), ... >
     */
    
    for (size_t i = 0; i < num_comps; ++i)
      output.splice(output.end(), reduce(*components[i].get()));
    
    // return < reduce(g1), reduce(g2), ... >
    return output;
  }
  
  RBGraph g_cm(g);
  std::list<RBVertex> cm = maximal_characters2(g_cm);
  maximal_reducible_graph(g_cm, cm);
  
  #ifdef DEBUG
  std::cout << "Cm = { ";
  std::list<RBVertex>::iterator i = cm.begin();
  for (; i != cm.end(); ++i) std::cout << g_cm[*i].name << " ";
  std::cout << "}" << std::endl
            << "Gcm:" << std::endl;
  print_rbgraph(g_cm);
  #endif
  
  // ...
  
  return output;
}

//=============================================================================
// Enum / Struct operator overloads

// General

std::ostream& operator<<(std::ostream& os, const State& s) {
  if (s == State::lose)
    os << "-";
  else
    os << "+";
  
  return os;
}

#include <algorithm>
#include "hdgraph.hpp"


//=============================================================================
// Boost functions (overloading)

// Hasse Diagram

HDVertex add_vertex(const std::list<std::string>& species,
                    const std::list<std::string>& characters, HDGraph& hasse) {
  const auto v = boost::add_vertex(hasse);
  hasse[v].species = species;
  hasse[v].characters = characters;
  hasse[boost::graph_bundle].num_v++;
  return v;
}

std::pair<HDEdge, bool> add_edge(
    const HDVertex u, const HDVertex v,
    const std::list<SignedCharacter>& signedcharacters, HDGraph& hasse) {
  HDEdge e;
  bool exists;
  std::tie(e, exists) = boost::add_edge(u, v, hasse);
  hasse[e].signedcharacters = signedcharacters;

  return std::make_pair(e, exists);
}

//=============================================================================
// General functions

// Hasse Diagram

std::ostream& operator<<(std::ostream& os, const HDGraph& hasse) {
  HDVertexIter v, v_end;
  std::tie(v, v_end) = vertices(hasse);
  for (; v != v_end; ++v) {
    os << "[ ";

    for (const auto& i : hasse[*v].species) {
      os << i << " ";
    }

    os << "( ";

    for (const auto& i : hasse[*v].characters) {
      os << i << " ";
    }

    os << ") ]:";
    HDOutEdgeIter e, e_end;
    std::tie(e, e_end) = out_edges(*v, hasse);
    for (; e != e_end; ++e) {
      const auto vt = target(*e, hasse);

      os << " -";

      auto j = hasse[*e].signedcharacters.cbegin();
      for (; j != hasse[*e].signedcharacters.cend(); ++j) {
        os << *j;

        if (std::next(j) != hasse[*e].signedcharacters.cend()) os << ",";
      }

      os << "-> [ ";

        for (const auto& i : hasse[vt].species) {
          os << i << " ";
        }

      os << "( ";

      for (const auto& i : hasse[vt].characters) {
        os << i << " ";
      }

      os << ") ];";
    }

    if (std::next(v) != v_end) os << std::endl;
  }

  return os;
}

//=============================================================================
// Algorithm functions

bool is_included(const std::list<std::string>& a,
                 const std::list<std::string>& b) {
  for (const auto& a_str : a) {
    if (std::find(b.cbegin(), b.cend(), a_str) == b.cend())
      // exit the function at the first string of a not present in b
      return false;
  }

  return true;
}



void hasse_diagram(HDGraph& hasse, const RBGraph& g, const RBGraph& gm) {
  std::vector<std::list<RBVertex>> vec_adj_char(num_species(gm));
  std::map<RBVertex, std::list<RBVertex>> adj_char;
  hasse[boost::graph_bundle].num_v = 0;
  // how vec_adj_char is going to be structured:
  // vec_adj_char[index] => < S, List of characters adjacent to S >

  // how adj_char is going to be structured:
  // adj_char[S] => < List of characters adjacent to S >

  // vec_adj_char is used to sort the lists by number of elements, this is why
  // we store the list of adjacent characters to S. While we store S to be able
  // to access adj_char[S] in costant time, instead of iterating on vec_adj_char
  // to find the correct list

  auto compare_names = [](const std::string& a, const std::string& b) {
    size_t a_index, b_index;
    std::stringstream ss;

    ss.str(a.substr(1));
    ss >> a_index;

    ss.clear();

    ss.str(b.substr(1));
    ss >> b_index;

    return a_index < b_index;
  };

  // initialize vec_adj_char and adj_char for each species in the graph
  RBVertexIter v, v_end;
  std::tie(v, v_end) = vertices(gm);
  for (size_t index = 0; v != v_end; ++v) {
    if (!is_species(*v, gm)) continue;
    // for each species vertex
    
    // vec_adj_char[index]'s first element is the species vertex
    vec_adj_char[index].push_back(*v);

    // build v's set of adjacent characters
    RBOutEdgeIter e, e_end;
    std::tie(e, e_end) = out_edges(*v, gm);
    for (; e != e_end; ++e) {
      //ignore active characters
      if(is_red(*e, gm)) continue;
      
      // vt = one of the characters adjacent to *v
      const auto vt = target(*e, gm);

      // vec_adj_char[index]'s other elements are the characters adjacent to S
      vec_adj_char[index].push_back(vt);
      // adj_char[S]'s elements are the characters adjacent to S
      adj_char[*v].push_back(vt);
    }

    // if the species *v would have 0 characters, ignore it
    if (vec_adj_char[index].size() == 1) {
      vec_adj_char[index].clear();

      continue;
    }

    index++;
  }

  auto compare_size = [](const std::list<RBVertex>& a,
                         const std::list<RBVertex>& b) {
    return a.size() < b.size();
  };

  // sort vec_adj_char by size in ascending order
  std::sort(vec_adj_char.begin(), vec_adj_char.end(), compare_size);

  bool first_iteration = true;
  for (const auto& set : vec_adj_char) {
    // for each set of characters
    if (set.empty()) continue;

    // v = species of gm
    const auto v = set.front();

    // fill the list of characters names of v
    std::list<std::string> lcv{};
    for (const auto& cv : adj_char[v]) {
      lcv.push_back(gm[cv].name);
    }

    lcv.sort(compare_names);

    if (first_iteration) {
      // first iteration of the loop:
      // add v to the Hasse diagram, and being the first vertex of the graph
      // there's no need to do any work
      add_vertex(gm[v].name, lcv, hasse);

      first_iteration = false;

      continue;
    }

    // new_edges will contain the list of edges that may be added to the Hasse
    // diagram: HDVertex is the source, std::string is the edge label
    std::list<std::pair<HDVertex, std::string>> new_edges;

    // check if there is a vertex with the same characters as v or if v needs
    // to be added to the Hasse diagram
    HDVertexIter hdv, hdv_end;
    std::tie(hdv, hdv_end) = vertices(hasse);
    for (; hdv != hdv_end; ++hdv) {
      // for each vertex in hasse

      if (lcv == hasse[*hdv].characters) {
        // v and hdv have the same characters

        // add v to the list of species in hdv
        hasse[*hdv].species.push_back(gm[v].name);

        break;
      }

      const std::list<std::string> lhdv = hasse[*hdv].characters;

      // initialize new_edges if lhdv is a subset of lcv, with the structure:
      // *hdv -*ci-> v
      if (is_included(lhdv, lcv)) {
        // hdv is included in v
        for (const auto& ci : lcv) {
          // for each character in hdv
          const auto in = std::find(lhdv.cbegin(), lhdv.cend(), ci);

          if (in == lhdv.end()) {
            // character is not present in lhdv
            new_edges.push_back(std::make_pair(*hdv, ci));
          }
        }
      }

      // on the last iteration of the cycle, after having collected all the
      // pairs in new_edges, add vertex and edges to the Hasse diagram
      if (std::next(hdv) == hdv_end) {
        // last iteration on the characters in the list has been performed

        // build a vertex for v and add it to the Hasse diagram
        const auto u = add_vertex(gm[v].name, lcv, hasse);

        // build in_edges for the vertex and add them to the Hasse diagram
        for (const auto& ei : new_edges) {
          // for each new edge to add to the Hasse diagram
          HDEdge edge;
          std::tie(edge, std::ignore) = add_edge(ei.first, u, hasse);
          hasse[edge].signedcharacters.push_back({ei.second, State::gain});
        }

        break;
      }
    }
  }

  // Store the graph pointer into the Hasse diagram's graph properties
  hasse[boost::graph_bundle].g = &g;

  // Store the maximal reducible graph pointer into the Hasse diagram's graph
  // properties
  hasse[boost::graph_bundle].gm = &gm;

  transitive_reduction(hasse); 

  // sort species names in each vertex
  HDVertexIter u, u_end;
  std::tie(u, u_end) = vertices(hasse);
  for (; u != u_end; ++u) {
    auto species = hasse[*u].species;

    species.sort(compare_names);

    hasse[*u].species.clear();
    hasse[*u].species.splice(hasse[*u].species.cend(), species);
  }
  reduce_diagram(hasse, gm);
}

void reduce_diagram(HDGraph& hasse, const RBGraph& gm){
  RBVertexIter rbv, rbv_end;  //Vertexes of the RBGraph
  RBOutEdgeIter rbe, rbe_end; //Edges of a vertex of RBGraph
  
  //List of species that must be deleted from the HDGraph
  std::list<std::string> ls;
  
  //Search for active species
  std::tie(rbv, rbv_end) = vertices(gm);
  while(*rbv != *rbv_end){
    if(!is_species(*rbv, gm)){
      rbv++;
      continue;
    }
    std::tie(rbe, rbe_end) = out_edges(*rbv, gm);
    while(*rbe != *rbe_end) {
      if(is_red(*rbe, gm)){
        ls.push_back(gm[*rbv].name);
        break;
      }
      rbe++;
    }
    rbv++;
  }  

  //Removes active species from Hasse vertexes
  HDVertexIter hdv, hdv_end;  //Hasse diagram vertexes
  std::tie(hdv, hdv_end) = vertices(hasse);

  while(hdv != hdv_end){  //For each vertex in Hasse diagram
    auto str = ls.begin(); //specie to remove 
    auto str_end = ls.end(); 
    while(str != str_end){
      hasse[*hdv].species.remove(*str);
      str++;    
    }
  hdv++;     
  }

  //Search for vertexes to remove  
  std::list<HDVertex> lvr;
  std::tie(hdv, hdv_end) = vertices(hasse);
  while(hdv != hdv_end){
    if(hasse[*hdv].species.size() == 0)
      lvr.push_back(*hdv);
    hdv++;
  }
  //Removing vertexes without species
  
  auto i = lvr.begin();
  auto i_end = lvr.end();

  while(i != i_end){ 
    i = lvr.begin();
    i_end = lvr.end(); 
    auto i_c = *i;
    i++;
    remove_vertex(i_c, hasse);
    lvr.pop_front();
  } 
}

void remove_vertex(HDVertex& v, HDGraph& hasse){
  HDInEdgeIter ie, ie_end;
  HDOutEdgeIter oe, oe_end;
  std::tie(ie, ie_end) = in_edges(v, hasse);

  while(ie != ie_end) {
    auto s = source(*ie, hasse);
    auto in_chars = hasse[*ie].signedcharacters;
    std::tie(oe, oe_end) = out_edges(v, hasse);
    while (oe != oe_end){
      auto t = target(*oe, hasse);
      auto out_chars = hasse[*oe].signedcharacters;
      for(const auto & elem : out_chars)
        in_chars.push_back(elem);
      
      HDEdge edge;
      bool exist;
      std::tie(edge, exist) = add_edge(s, t, hasse);
      if(exist)
        hasse[edge].signedcharacters = in_chars;
      oe++;
    }
    ie++;
  }
  boost::clear_vertex(v, hasse);
  boost::remove_vertex(v, hasse);
  hasse[boost::graph_bundle].num_v--;
}

void transitive_reduction(HDGraph& hasse) {
  HDVertexIter u, u_end;
  std::tie(u, u_end) = vertices(hasse);
  for (; u != u_end; ++u) {
    if (in_degree(*u, hasse) == 0 || out_degree(*u, hasse) == 0) continue;
    // for each internal vertex in hasse

    HDInEdgeIter ie, ie_end;
    std::tie(ie, ie_end) = in_edges(*u, hasse);
    for (; ie != ie_end; ++ie) {
      // for each in-edge
      HDOutEdgeIter oe, oe_end;
      std::tie(oe, oe_end) = out_edges(*u, hasse);
      for (; oe != oe_end; ++oe) {
        // for each out-edge
        // source -> u -> target
        HDEdge e;
        bool exists;
        std::tie(e, exists) =
            edge(source(*ie, hasse), target(*oe, hasse), hasse);

        if (!exists)
          // no transitivity between source and target
          continue;

        // remove source -> target, which breaks the no-transitivty rule in
        // the Hasse diagram we need, because a path between source and target
        // already exists in the form: source -> u -> target
        remove_edge(e, hasse);
      }
    }
  }
}

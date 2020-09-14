#include "rbgraph.hpp"
#include <boost/graph/connected_components.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/graph_utility.hpp>
#include <fstream>

//=============================================================================
// Boost functions (overloading)

void remove_vertex(const RBVertex& v, RBGraph& g) {

  if (!exists(v, g)) {
    throw std::runtime_error("[ERROR: remove_vertex()] The input RBVertex does not exist");
  } 

  // delete v from the map
  vertex_map(g).erase(g[v].name);

  boost::clear_vertex(v, g);
  boost::remove_vertex(v, g);

  if (is_species(v, g))
    num_species(g)--;
  else
    num_characters(g)--;
}


void remove_vertex(const std::string& name, RBGraph& g) {
  remove_vertex(get_vertex(name, g), g);
}


RBVertex add_vertex(const std::string& name, const Type type, RBGraph& g) {
  const auto u = vertex_map(g).find(name);
  if (u != vertex_map(g).end()) {
    throw std::runtime_error("[ERROR: add_vertex()] RBVertex with name \"" + name + "\" already exists");
  } 

  const RBVertex v = boost::add_vertex(g);

  // insert v in the map
  vertex_map(g)[name] = v;

  g[v].name = name;
  g[v].type = type;

  if (is_species(v, g))
    num_species(g)++;
  else
    num_characters(g)++;

  return v;
}

std::pair<RBEdge, bool> add_edge(const RBVertex& u, const RBVertex& v,
                                 const Color color, RBGraph& g) {
  if (!exists(u, g) || !exists(v, g))
    throw std::runtime_error("[ERROR: add_edge()] One or both the input RBVerteces do not exist");
  RBEdge e;
  bool exists;
  std::tie(e, exists) = boost::add_edge(u, v, g);
  g[e].color = color;

  return std::make_pair(e, exists);
}

RBEdge get_edge(const RBVertex &source, const RBVertex &target, RBGraph &g) {
  if (!exists(source, g) || !exists(target, g))
    throw std::runtime_error("[ERROR: get_edge()] One or both the input RBVerteces do not exist in the RBGraph");

  RBEdge e;
  bool exists;
  std::tie(e, exists) = boost::edge(source, target, g);
  if (!exists) {
    std::string name_source = g[source].name;
    std::string name_target = g[target].name;
    throw std::runtime_error("[ERROR: get_edge()] edge with source=\"" + name_source + "\" and target=\"" + name_target + "\" does not exist");
  }

  return e;
}

//=============================================================================
// General functions

const RBVertex& get_vertex(const std::string& name, const RBGraph& g) {
  try {
    return vertex_map(g).at(name);
  } catch (std::out_of_range) {
    throw std::runtime_error("[ERROR: get_vertex()] RBVertex with name \"" + name + "\" does not exist in the vertex map of the RBGraph");
  }
}

bool exists(const RBVertex &source, const RBVertex &target, RBGraph &g) {
  if (!exists(source, g) || !exists(target, g))
    return false;
  
  RBOutEdgeIter e, e_end;
  std::tie(e, e_end) = out_edges(source, g);
  for (; e != e_end; ++e) {
    if (g[(*e).m_target].name == g[target].name &&
      g[(*e).m_target].type == g[target].type)
      return true;
  }
  return false;
}

bool exists(const std::string &source, const std::string &target, RBGraph &g) {
  if (!exists(source, g) || !exists(target, g))
    return false;
  return exists(get_vertex(source, g), get_vertex(target, g), g);
}

bool exists(const RBVertex &v, RBGraph &g) {
  RBVertexIter u, u_end;
  std::tie(u, u_end) = vertices(g);
  for (; u != u_end; ++u) {
    if (*u == v)
      return true;
  }
  return false;
}

bool exists(const std::string &name, RBGraph &g) {
  RBVertexIter u, u_end;
  std::tie(u, u_end) = vertices(g);
  for (; u != u_end; ++u) {
    if (g[*u].name == name)
      return true;
  }
  return false;
}

void build_vertex_map(RBGraph& g) {
  vertex_map(g).clear();

  RBVertexIter v, v_end;
  std::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    vertex_map(g)[g[*v].name] = *v;
  }
}

void copy_graph(const RBGraph& g, RBGraph& g_copy) {
  RBVertexIMap index_map;
  RBVertexIAssocMap index_assocmap(index_map);

  // fill the vertex index map index_assocmap
  RBVertexIter u, u_end;
  std::tie(u, u_end) = vertices(g);
  for (size_t index = 0; u != u_end; ++u, ++index) {
    boost::put(index_assocmap, *u, index);
  }

  // copy g to g_copy
  copy_graph(g, g_copy, boost::vertex_index_map(index_assocmap));

  // update g_copy's number of species and characters
  num_species(g_copy) = num_species(g);
  num_characters(g_copy) = num_characters(g);

  // rebuild g_copy's map
  build_vertex_map(g_copy);
}

void copy_graph(const RBGraph& g, RBGraph& g_copy, RBVertexMap& v_map) {
  RBVertexIMap index_map;
  RBVertexAssocMap v_assocmap(v_map);
  RBVertexIAssocMap index_assocmap(index_map);

  // fill the vertex index map index_assocmap
  RBVertexIter u, u_end;
  std::tie(u, u_end) = vertices(g);
  for (size_t index = 0; u != u_end; ++u, ++index) {
    boost::put(index_assocmap, *u, index);
  }

  // copy g to g_copy, fill the vertex map v_assocmap (and v_map)
  copy_graph(g, g_copy,
             boost::vertex_index_map(index_assocmap).orig_to_copy(v_assocmap));

  // update g_copy's number of species and characters
  num_species(g_copy) = num_species(g);
  num_characters(g_copy) = num_characters(g);

  // rebuild g_copy's map
  build_vertex_map(g_copy);
}

std::ostream& operator<<(std::ostream& os, const RBGraph& g) {
  std::list<std::string> lines;
  std::list<std::string> species;
  std::list<std::string> characters;

  RBVertexIter v, v_end;
  std::tie(v, v_end) = vertices(g);
  for (size_t i = 0; v != v_end; ++v, ++i) {
    std::list<std::string> edges;

    RBOutEdgeIter e, e_end;
    std::tie(e, e_end) = out_edges(*v, g);
    for (size_t j = 0; e != e_end; ++e, ++j) {
      std::string edge;
      edge += " -";
      edge += (is_red(*e, g) ? "r" : "-");
      edge += "- ";
      edge += g[target(*e, g)].name;
      edge += ";";

      edges.push_back(edge);
    }

    auto compare_edges = [](const std::string& a, const std::string& b) {
      size_t a_index, b_index;
      std::stringstream ss;

      ss.str(a.substr(6, a.size() - 7));
      ss >> a_index;

      ss.clear();

      ss.str(b.substr(6, b.size() - 7));
      ss >> b_index;

      return a_index < b_index;
    };

    edges.sort(compare_edges);

    std::string edges_str;
    for (const auto& edge : edges) {
      edges_str.append(edge);
    }

    auto line(g[*v].name + ":" + edges_str);

    if (std::next(v) != v_end) line += "\n";

    if (is_species(*v, g))
      species.push_back(line);
    else
      characters.push_back(line);
  }

  auto compare_lines = [](const std::string& a, const std::string& b) {
    size_t a_index, b_index;
    std::stringstream ss;

    ss.str(a.substr(1, a.find(":")));
    ss >> a_index;

    ss.clear();

    ss.str(b.substr(1, b.find(":")));
    ss >> b_index;

    return a_index < b_index;
  };

  species.sort(compare_lines);
  characters.sort(compare_lines);

  lines.splice(lines.end(), species);
  lines.splice(lines.end(), characters);

  std::string lines_str;
  for (const auto& line : lines) {
    lines_str += line;
  }

  os << lines_str;

  return os;
}

// File I/O

void read_graph(const std::string& filename, RBGraph& g) {
  std::vector<RBVertex> species, characters;
  bool first_line = true;
  std::string line;
  std::ifstream file(filename);
  std::vector<std::string> a_chars;

  if (!file) {
    // input file doesn't exist
    throw std::runtime_error(
        "[ERROR: read_graph()] Failed to read graph from file: no such file or directory");
  }

  size_t index = 0;
  while (std::getline(file, line)) {
    // for each line in file
    std::istringstream iss(line);

    if (first_line) {
      size_t cont = 0;
      size_t read;
      size_t num_s, num_c;

      while(iss >> read) {
        if(cont == 0){
          num_s = read;
          cont++;
        }
        else if(cont == 1) {
          num_c = read;
          cont++;
        }
        else {
          if(read >= num_c)
            throw std::runtime_error("[ERROR: read_graph()] Failed to read graph from file: Inexistent character");
          std::string s = "c" + std::to_string(read);
          a_chars.push_back(s);
        }
      }

      species.resize(num_s);
      characters.resize(num_c);

      if (species.size() == 0 || characters.size() == 0) {
        // input file parsing error
        throw std::runtime_error(
            "[ERROR: read_graph()] Failed to read graph from file: badly formatted line 0");
      }

      // insert species in the graph
      for (size_t j = 0; j < species.size(); ++j) {
        const auto v_name = "s" + std::to_string(j);

        species[j] = add_species(v_name, g);
      }

      // insert characters in the graph
      for (size_t j = 0; j < characters.size(); ++j) {
        const auto v_name = "c" + std::to_string(j);

        characters[j] = add_character(v_name, g);
      }

      first_line = false;
    } 
    else {
      char value;

      // read binary matrix
      while (iss >> value) {
        switch (value) {
          case '1':
            // add edge between species[s_index] and characters[c_index]
            {
              const auto s_index = index / characters.size(),
                         c_index = index % characters.size();

              if (s_index >= species.size() || c_index >= characters.size()) {
                // input file parsing error
                throw std::runtime_error(
                    "[ERROR: read_graph()] Failed to read graph from file: oversized matrix");
              }

              RBEdge edge;
              std::tie(edge, std::ignore) =
                  add_edge(species[s_index], characters[c_index], g);
            }
            break;

          case '0':
            // ignore
            break;

          default:
            // input file parsing error
            throw std::runtime_error(
                "[ERROR: read_graph()] Failed to read graph from file: unexpected value in matrix");
        }

        index++;
      }
    }
  }

  

  if (index != species.size() * characters.size()) {
    // input file parsing error
    throw std::runtime_error(
        "[ERROR: read_graph()] Failed to read graph from file: undersized matrix");
  }

  if (species.size() == 0 || characters.size() == 0) {
    // input file parsing error
    throw std::runtime_error("[ERROR: read_graph()] Failed to read graph from file: empty file");
  }

  for(const auto& elem : a_chars)
    change_char_type(vertex_map(g).at(elem), g);
}

//=============================================================================
// Algorithm functions

bool is_active(const RBVertex& v, const RBGraph& g) {
  if (!is_character(v, g)) 
    return false;

  RBOutEdgeIter e, e_end;
  std::tie(e, e_end) = out_edges(v, g);
  for (; e != e_end; ++e)
    if (!is_red(*e, g)) 
      return false;

  return true;
}

bool is_inactive(const RBVertex& v, const RBGraph& g) {
  if (!is_character(v, g)) 
    return false;

  RBOutEdgeIter e, e_end;
  std::tie(e, e_end) = out_edges(v, g);
  for (; e != e_end; ++e)
    if (!is_black(*e, g)) 
    return false;

  return true;
}

void remove_singletons(RBGraph& g) {
  RBVertexIter v, v_end, next;
  std::tie(v, v_end) = vertices(g);
  for (next = v; v != v_end; v = next) {
    next++;
    remove_vertex_if(*v, if_singleton(), g);
  }
}

bool is_free(const RBVertex& v, const RBGraph& g) {
  if (!is_character(v, g)) 
    return false;

  RBVertexIMap index_map, comp_map;
  RBVertexIAssocMap index_assocmap(index_map), comp_assocmap(comp_map);
  
  // fill vertex index map
  RBVertexIter u, u_end;
  std::tie(u, u_end) = vertices(g);
  for (size_t index = 0; u != u_end; ++u, ++index) {
    boost::put(index_assocmap, *u, index);
  }

  // build the components map
  boost::connected_components(g, comp_assocmap,
                              boost::vertex_index_map(index_assocmap));

  return is_free(v, g, comp_map);
}

bool is_free(const RBVertex& v, const RBGraph& g, const RBVertexIMap& c_map) {
  if (!is_character(v, g)) 
    return false;

  size_t tot_species = 0;
  RBVertexIter u, u_end;
  std::tie(u, u_end) = vertices(g);
  for (; u != u_end; ++u) {
    // if it is not a species or it is not located in the component of v
    if (!is_species(*u, g) || c_map.at(v) != c_map.at(*u)) {
      continue;
    }
    tot_species++;
  }

  size_t count_species = 0;
  RBOutEdgeIter e, e_end;
  std::tie(e, e_end) = out_edges(v, g);
  for (; e != e_end; ++e) {
    if (!is_red(*e, g)) 
      return false;
    count_species++;
  }

  if (count_species != tot_species) 
    return false;

  return true;
}

bool is_universal(const RBVertex v, const RBGraph& g) {
  if (!is_character(v, g)) return false;

  RBVertexIMap index_map, comp_map;
  RBVertexIAssocMap index_assocmap(index_map), comp_assocmap(comp_map);

  // fill vertex index map
  RBVertexIter u, u_end;
  std::tie(u, u_end) = vertices(g);
  for (size_t index = 0; u != u_end; ++u, ++index) {
    boost::put(index_assocmap, *u, index);
  }

  // build the components map
  boost::connected_components(g, comp_assocmap,
                              boost::vertex_index_map(index_assocmap));

  return is_universal(v, g, comp_map);
}

bool is_universal(const RBVertex v, const RBGraph& g,
                  const RBVertexIMap& c_map) {
  if (!is_character(v, g)) return false;

  size_t tot_species = 0;

  RBVertexIter u, u_end;
  std::tie(u, u_end) = vertices(g);
  for (; u != u_end; ++u) {
    if (!is_species(*u, g) || c_map.at(v) != c_map.at(*u)) continue;

    tot_species++;
  }

  size_t count_species = 0;

  RBOutEdgeIter e, e_end;
  std::tie(e, e_end) = out_edges(v, g);
  for (; e != e_end; ++e) {
    if (!is_black(*e, g) || !is_species(target(*e, g), g)) return false;

    count_species++;
  }

  if (count_species != tot_species) return false;

  return true;
}

RBGraphVector connected_components(const RBGraph& g) {
  RBVertexIMap index_map, comp_map;
  RBVertexIAssocMap index_assocmap(index_map), comp_assocmap(comp_map);

  // fill the vertex index map index_assocmap
  RBVertexIter v, v_end;
  std::tie(v, v_end) = vertices(g);
  for (size_t index = 0; v != v_end; ++v, ++index) {
    boost::put(index_assocmap, *v, index);
  }

  // get number of components and the components map
  size_t comp_count = boost::connected_components(
      g, comp_assocmap, boost::vertex_index_map(index_assocmap));

  // how comp_map is structured (after running boost::connected_components):
  // comp_map[i] => < vertex_in_g, component_index >
  return connected_components(g, comp_map, comp_count);
}

RBGraphVector connected_components(const RBGraph& g, const RBVertexIMap& c_map,
                                   const size_t c_count) {
  RBGraphVector components;
  RBVertexMap vertices;
  
  // how vertices is going to be structured:
  // vertices[vertex_in_g] => vertex_in_component

  // resize subgraph components
  components.resize(c_count);

  // initialize subgraph components
  for (size_t i = 0; i < c_count; ++i) {
    components[i] = std::make_unique<RBGraph>();
  }
  
  if (c_count <= 1) {
    // graph is connected
    return components;
  }
  // graph is disconnected

  // add vertices to their respective subgraph
  for (const auto& vcomp : c_map) {
    // for each vertex
    const auto v = vcomp.first;
    const auto comp = vcomp.second;
    auto* const component = components[comp].get();

    // add the vertex to *component and copy its descriptor in vertices[v]
    vertices[v] = add_vertex(g[v].name, g[v].type, *component);
  }

  // add edges to their respective vertices and subgraph
  for (const auto& vcomp : c_map) {
    // for each vertex
    const auto v = vcomp.first;

    // prevent duplicate edges from characters to species
    if (!is_species(v, g)) continue;

    const auto new_v = vertices[v];
    const auto comp = vcomp.second;
    auto* const component = components[comp].get();

    RBOutEdgeIter e, e_end;
    std::tie(e, e_end) = out_edges(v, g);
    for (; e != e_end; ++e) {
      // for each out edge
      const auto new_vt = vertices[target(*e, g)];

      bool exists;
      std::tie(std::ignore, exists) = edge(new_v, new_vt, *component);

      // prevent duplicate edges on non-bipartite graphs
      if (exists) continue;

      add_edge(new_v, new_vt, g[*e].color, *component);
    }
  }

  if (logging::enabled) {
    
    if (c_count == 1) {
      std::cout << "G connected" << std::endl;
    } else {
      std::cout << "Connected components: " << c_count << std::endl;

      for (const auto& component : components) {
        std::cout << *component.get() << std::endl << std::endl;
      }
    }
  }

  return components;
}

const std::list<RBVertex> maximal_characters(const RBGraph& g) {
  std::list<RBVertex> cm;
  std::map<RBVertex, std::list<RBVertex>> adj_spec;

  // how adj_spec is going to be structured:
  // adj_spec[C] => < List of adjacent species to C >

  RBVertexIter v, v_end;
  std::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    if (!is_character(*v, g)) continue;
    // for each character vertex

    // build v's set of adjacent species
    RBOutEdgeIter e, e_end;
    std::tie(e, e_end) = out_edges(*v, g);
    for (; e != e_end; ++e) {
      RBVertex vt = target(*e, g);

      // if v is active or connected to random nodes ignore it
      if ((!active::enabled && is_red(*e, g)) || !is_species(vt, g)) break;

      adj_spec[*v].push_back(vt);
    }

    if (cm.empty()) {
      // cm being empty means v can be added without further checks
      cm.push_back(*v);
      continue;
    }

    // adj_spec[*v] now contains the list of species adjacent to v

    bool skip_cycle = false;
    bool subst = false;

    // check if adj_spec[*v] is subset of the species adjacent to cmv
    RBVertexIter cmv = cm.begin(), cmv_end = cm.end();
    for (; cmv != cmv_end; ++cmv) {
      // for each species in cm
      if (skip_cycle) break;

      size_t count_incl = 0;
      size_t count_excl = 0;
      bool keep_char = false;

      RBVertexIter sv = adj_spec[*v].begin(), sv_end = adj_spec[*v].end();
      for (; sv != sv_end; ++sv) {
        // for each species adjacent to v, S(C#)

        // find sv in the list of cmv's adjacent species
        std::list<RBVertex>::const_iterator in =
            std::find(adj_spec[*cmv].cbegin(), adj_spec[*cmv].cend(), *sv);

        // keep count of how many species are included (or not found) in
        // the list of cmv's adjacent species
        if (in != adj_spec[*cmv].end())
          count_incl++;
        else
          count_excl++;

        if (std::next(sv) == sv_end) {
          // last iteration on the species in the list has been performed
          if (count_incl == adj_spec[*cmv].size() && count_excl > 0) {
            // the list of adjacent species to v is a superset of the list of
            // adjacent species to cmv, which means cmv can be replaced
            // by v in the list of maximal characters Cm
            if (subst) {
              cm.remove(*(cmv++));
            } else {
              cm.push_front(*v);
              cm.remove(*(cmv++));

              subst = true;
            }

            cmv--;
          } else if (count_incl < adj_spec[*cmv].size() && count_excl > 0) {
            // the list of adjacent species to v is neither a superset nor a
            // subset of the list of adjacent species to cmv, which means
            // v may be a new maximal character
            if (!subst) keep_char = true;
          } else if (count_incl == adj_spec[*cmv].size()) {
            // the list of adjacent species to v is the same as the list of
            // adjacent species to cmv, so v can be ignored in the next
            // iterations on the characters in Cm
            skip_cycle = true;
          } else if (count_excl == 0) {
            // the list of adjacent species to v is a subset of the list of
            // adjacent species to cmv, meaning v can be ignored in the
            // next iterations on the characters in Cm
            skip_cycle = true;
          }
        }
      }

      if (std::next(cmv) == cmv_end) {
        // last iteration on the characters in the list has been performed
        if (keep_char) {
          // after all the iterations keep_char is true if the list of adjacent
          // species to v is neither a superset nor a subset of the lists of
          // adjacent species to the characters in Cm, so v is a maximal
          // characters and can be added to the list of maximal characters Cm
          cm.push_front(*v);
        }
      }
    }
  }

  cm.reverse();

  return cm;
}

RBGraph maximal_reducible_graph(const RBGraph& g, const bool active) {
  // copy g to gm
  RBGraph gm;
  copy_graph(g, gm);

  // compute the maximal characters of gm
  const auto cm = maximal_characters(gm);

  if (logging::enabled) {
    // verbosity enabled
    std::cout << "Maximal characters Cm = { ";

    for (const auto& kk : cm) {
      std::cout << gm[kk].name << " ";
    }

    std::cout << "} - Count: " << cm.size() << std::endl;
  }

  // remove non-maximal characters of gm
  RBVertexIter v, v_end, next;
  std::tie(v, v_end) = vertices(gm);
  for (next = v; v != v_end; v = next) {
    next++;

    if (!is_character(*v, gm))
      // don't remove non-character vertices
      continue;

    if (active && is_active(*v, gm))
      // don't remove active or non-character vertices
      continue;

    remove_vertex_if(*v, if_not_maximal(cm), gm);
  }

  remove_singletons(gm);

  return gm;
}

bool has_red_sigmagraph(const RBGraph& g) {
  size_t count_actives = 0;

  RBVertexIter v, v_end;
  std::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    if (is_active(*v, g)) count_actives++;

    if (count_actives == 2) break;
  }

  // if count_actives doesn't reach 2, g can't contain a red sigma-graph
  if (count_actives < 2) return false;

  std::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    if (!is_active(*v, g)) continue;
    // for each active character

    for (auto u = std::next(v); u != v_end; ++u) {
      if (!is_active(*u, g)) continue;
      // for each active character coming after *v

      // check if characters *v and *u are part of a red sigma-graph
      if (has_red_sigmapath(*v, *u, g)) return true;
    }
  }

  return false;
}

bool has_red_sigmapath(const RBVertex c0, const RBVertex c1, const RBGraph& g) {
  // vertex that connects c0 and c1 (always with red edges)
  RBVertex junction = 0;

  bool half_sigma = false;

  RBOutEdgeIter e, e_end;
  std::tie(e, e_end) = out_edges(c0, g);
  for (; e != e_end; ++e) {
    if (!is_red(*e, g)) continue;

    auto s = target(*e, g);

    // for each species connected to c0 via a red edge

    RBEdge edgec1;
    bool existsc1;
    std::tie(edgec1, existsc1) = edge(c1, s, g);

    // check if s can be a junction vertex
    if (junction == 0 && existsc1 && is_red(edgec1, g)) {
      junction = s;

      continue;
    }

    // check if s and c1 are connected
    if (existsc1)
      // skip s
      continue;

    half_sigma = true;

    if (junction != 0) break;
  }

  if (!half_sigma || junction == 0) return false;

  std::tie(e, e_end) = out_edges(c1, g);
  for (; e != e_end; ++e) {
    RBVertex s = target(*e, g);

    if (!is_red(*e, g) || s == junction) continue;

    // for each species connected to c1 via a red edge (which is not junction)

    RBEdge edgec0;
    bool existsc0;
    std::tie(edgec0, existsc0) = edge(c0, s, g);

    // check if s and c0 are connected
    if (existsc0)
      // skip s
      continue;

    return true;
  }

  return false;
}

void change_char_type(const RBVertex& v, RBGraph& g) {
  RBOutEdgeIter e, e_end;
  std::tie(e, e_end) = out_edges(v, g);
  
  for(; e != e_end; ++e)
    if(is_red(*e, g))
      g[*e].color = Color::black;
    else
      g[*e].color = Color::red;
}

std::set<std::string> active_characters(const RBGraph& g) {
  std::set<std::string> ac;
  RBVertexIter v, v_end;
  
  std::tie(v, v_end) = vertices(g);
  while(v != v_end) {
    if(is_active(*v, g))
      ac.insert(g[*v].name);
    v++;
  }
  return ac;
}

std::set<std::string> specie_active_characters(const RBVertex v, const RBGraph& g) {
  std::set<std::string> s{};
  if(is_character(v, g)) return s;

  RBOutEdgeIter oe, oe_end;
  std::tie(oe, oe_end) = out_edges(v, g);
  while(oe != oe_end) {
    if(g[*oe].color == Color::red)
      s.insert(g[target(*oe, g)].name);
    oe++;
  }
  return s;

}

std::set<std::string> comp_active_characters(const RBVertex v, const RBGraph& g) {
  if (is_character(v, g)) return {};

  RBVertexIMap index_map, comp_map;
  RBVertexIAssocMap index_assocmap(index_map), comp_assocmap(comp_map);

  // fill vertex index map
  RBVertexIter u, u_end;
  std::tie(u, u_end) = vertices(g);
  for (size_t index = 0; u != u_end; ++u, ++index) {
    boost::put(index_assocmap, *u, index);
  }

  // build the components map
  boost::connected_components(g, comp_assocmap,
                              boost::vertex_index_map(index_assocmap));
  return comp_active_characters(v, g, comp_map);
}

std::set<std::string> comp_active_characters(const RBVertex v, const RBGraph& g, const RBVertexIMap& c_map) {
  if (is_character(v, g)) return {};
  std::set<std::string> ac;

  RBVertexIter u, u_end;
  std::tie(u, u_end) = vertices(g);
  for (; u != u_end; ++u) {
    if(!is_active(*u, g) || c_map.at(v) != c_map.at(*u)) continue; 
    ac.insert(g[*u].name);
  }

  return ac;
}
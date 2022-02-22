/** 
 * 
 * @file functions.cpp
 * 
 * @author Simone Paolo Mottadelli
 * 
 */

#include "functions.hpp"
#include "rbgraph.hpp"
#include <boost/graph/connected_components.hpp>
#include <boost/graph/depth_first_search.hpp>


std::pair<std::list<SignedCharacter>, bool> realize_character(const SignedCharacter& sc, RBGraph& g) {
  std::list<SignedCharacter> output;

  // current character vertex
  RBVertex cv = 0;

  // get the vertex in g whose name is sc.character
  try {
    cv = get_vertex(sc.character, g);
  } catch (const std::out_of_range& e) {
    // g has no vertex named sc.character
    return std::make_pair(output, false);
  }

  RBVertexIMap i_map, c_map;
  RBVertexIAssocMap i_assocmap(i_map), c_assocmap(c_map);

  // fill vertex index map
  RBVertexIter v, v_end;
  std::tie(v, v_end) = vertices(g);
  for (size_t index = 0; v != v_end; ++v, ++index) {
    boost::put(i_assocmap, *v, index);
  }

  // build the components map
  boost::connected_components(g, c_assocmap,
                              boost::vertex_index_map(i_assocmap));
  
  if (sc.state == State::gain && is_inactive(cv, g)) {
    // c+ and c is inactive
    if (logging::enabled) {
      // verbosity enabled
      // std::cout << "Realizing " << sc;
    }
    // realize the character c+:
    // - add a red edge between c and each species in D(c) \ N(c)
    // - delete all black edges incident on c
    std::tie(v, v_end) = vertices(g);
    for (; v != v_end; ++v) {
      if (!is_species(*v, g) || c_map.at(*v) != c_map.at(cv)) continue;
      // for each species in the same connected component of cv
      
      RBEdge e;
      bool exists;
      std::tie(e, exists) = edge(*v, cv, g);

      if (exists)
        // there is an edge (black) between *v and cv
        remove_edge(e, g);
      else
        // there isn't an edge between *v and cv
        add_edge(*v, cv, Color::red, g);
    }

    if (logging::enabled) {
      // verbosity enabled
      //std::cout << std::endl;
    }
  } else if (sc.state == State::lose && is_active(cv, g)) {
    // c- and c is active
    if (logging::enabled) {
      // verbosity enabled
      //std::cout << "Realizing " << sc << std::endl;
    }

    // realize the character c- if it is connected through red edges to all the species of the component in which c resides. In this case:
    // - delete all edges incident on c
    bool connected = true;
    std::tie(v, v_end) = vertices(g);
    for (; v != v_end; ++v) {
      if (!is_species(*v, g) || c_map.at(*v) != c_map.at(cv)) 
        continue;

      // if cv is not connected to the species v, which belongs to its same component, then cv is not connected to all the species of its component.
      if (!exists(*v, cv, g)) {
        connected = false;
        break;
      }
    }

    if (connected)
      clear_vertex(cv, g);
    else {
      if (logging::enabled) {
          // verbosity enabled
          //std::cout << "Could not realize " << sc << std::endl;
      }
      return std::make_pair(output, false);
    }
  } else {
    if (logging::enabled) {
      // verbosity enabled
      //std::cout << "Could not realize " << sc << std::endl;
    }

    // this should never happen during the algorithm, but it is handled just in
    // case something breaks (or user input happens)
    return std::make_pair(output, false);
  }

  output.push_back(sc);

  // delete all isolated vertices
  remove_singletons(g);

  return std::make_pair(output, true);
}

std::pair<std::list<SignedCharacter>, bool> realize_species(const RBVertex v,
                                                    RBGraph& g) {
  std::list<SignedCharacter> lsc;

  if (!is_species(v, g)) 
    return std::make_pair(lsc, false);

  // build the list of inactive characters adjacent to v (species)
  
  std::list<RBVertex> adjacent_chars = get_adj_vertices(v, g);

  for (RBVertex c : adjacent_chars) 
    if (is_inactive(c, g))
      lsc.push_back({g[c].name, State::gain});
  
  return realize(lsc, g);
}

std::pair<std::list<SignedCharacter>, bool> realize(
    const std::list<SignedCharacter>& lsc, RBGraph& g) {
  std::list<SignedCharacter> output;

  // realize the list of signed characters lsc; the algorithm stops when a
  // non-feasible realization is encountered, setting the boolean flag to false
  // TODO: maybe change this behaviour
  for (const SignedCharacter i : lsc) {
    if (std::find(output.cbegin(), output.cend(), i) != output.cend())
      // the signed character i has already been realized in a previous sc
      continue;

    std::list<SignedCharacter> sc;
    bool feasible;

    std::tie(sc, feasible) = realize_character(i, g);

    if (!feasible) return std::make_pair(sc, false);

    output.splice(output.cend(), sc);
  }

  return std::make_pair(output, true);
}

bool is_complete(std::list<SignedCharacter> sc, const RBGraph& gm) {
  RBVertexIter v, v_end;
  auto scb = sc.begin();
  auto sce = sc.end();
  std::tie(v, v_end) = vertices(gm);
  while(v != v_end){
    if(is_inactive(*v, gm)){
      while(scb != sce){
        if((get_vertex(scb->character, gm) == *v))
          return false;
        scb++;
      }
    }
    v++;
  }
  return true;
}

void sort_by_degree_desc(std::list<RBVertex>& list_to_sort, const RBGraph& g) {
  // constructing a list of pairs <RBVertex, int>, where the first element is a vertex and the second element is the degree of the vertex
  std::list<std::pair<RBVertex, int>> list_of_pairs;
  
  for (RBVertex v : list_to_sort)
    list_of_pairs.push_back(std::make_pair(v, out_degree(v, g)));
  
  auto comparator = [](const std::pair<RBVertex, int> &a, const std::pair<RBVertex, int> &b) { 
    return  a.second > b.second; 
  };

  list_of_pairs.sort(comparator);

  list_to_sort.clear();

  for (std::pair<RBVertex, int> pair : list_of_pairs)
    list_to_sort.push_back(pair.first);
}

void sort_by_degree_asc(std::list<RBVertex>& list_to_sort, const RBGraph& g) {
  // constructing a list of pairs <RBVertex, int>, where the first element is a vertex and the second element is the degree of the vertex
  std::list<std::pair<RBVertex, int>> list_of_pairs;
  
  for (RBVertex v : list_to_sort)
    list_of_pairs.push_back(std::make_pair(v, out_degree(v, g)));
  
  auto comparator = [](const std::pair<RBVertex, int> &a, const std::pair<RBVertex, int> &b) { 
    return  a.second < b.second; 
  };

  list_of_pairs.sort(comparator);

  list_to_sort.clear();

  for (std::pair<RBVertex, int> pair : list_of_pairs)
    list_to_sort.push_back(pair.first);
}

std::list<RBVertex> get_all_minimal_p_active_species(const RBGraph& g, bool all) {
  std::list<RBVertex> out;
  std::list<RBVertex> active_species = get_active_species(g);
  sort_by_degree_desc(active_species, g);

  bool found;
  int num_inctv_chars_v, num_inctv_chars_u;
  for (RBVertex v : active_species) {
    found = false;
    // for every active species v in the sorted list of active species
    for (int i = 1; i < num_characters(g); ++i) {
      // index "i" is used after to check if vertex "u" has "i" more inactive characters than "v"

      for (RBVertex u : get_neighbors(v, g)) {
        if (u == v || is_character(u, g)) continue;
        // for every species u (neighbor of v)

        if (includes_species(u, v, g)) {
          // if u includes all the inactive species of v, then check if u has "i" species more than v
          num_inctv_chars_v = get_adj_inactive_characters(v, g).size();
          num_inctv_chars_u = get_adj_inactive_characters(u, g).size();
          if (num_inctv_chars_u == num_inctv_chars_v + i) {

            // check if the realization of v and then of u can generate any red-sigmagraphs in g
            RBGraph g_copy;
            copy_graph(g, g_copy);
            
            realize_species(get_vertex(g[v].name, g_copy), g_copy);
            realize_species(get_vertex(g[u].name, g_copy), g_copy);
            if (!has_red_sigmagraph(g_copy)) {
              out.push_back(v);
              found = true;
              break;
            }
          }
        }
      }
      if (found) break;
    }
    if (found && !all)
      break;
  }
  return out;
}

RBVertex get_minimal_p_active_species(const RBGraph& g) {
  return *get_all_minimal_p_active_species(g, false).begin();
}

RBVertex get_quasi_active_species(const RBGraph& g) {
  for (RBVertex v : g.m_vertices) {
      if (!is_species(v, g)) continue;
      RBOutEdgeIter e, e_end;
      std::tie(e, e_end) = out_edges(v, g);
      int black_edges = 0;
      int red_edges = 0;
      for (; e != e_end; ++e)
        if (is_black(*e, g))
          ++black_edges;
        else
          ++red_edges;
      // if we are here then s is a species with some red incoming edges. So, we have to check whether its realization generates a red-sigmagraph
      if (black_edges > 0 && red_edges > 0) {
        RBGraph g_copy;
        copy_graph(g, g_copy);
        realize_species(get_vertex(g[v].name, g_copy), g_copy);
        if (!has_red_sigmagraph(g_copy))
          return v;
      }
  }
  return 0;
}

RBVertex get_pending_and_minimal_p_active_species_with_unique_closest_neighbor(const RBGraph& g) {

  RBVertex result = 0;
  std::list<RBVertex> neighbors;
  std::list<RBVertex> minimal_p_active_species = get_all_minimal_p_active_species(g, true);

  for (RBVertex v :  get_pending_species(g)) {
    neighbors = get_closest_neighbors(v, g);

    if (neighbors.size() == 1 && contains(minimal_p_active_species, v))
      return v;
  }

  return result;
}

std::list<SignedCharacter> ppp_maximal_reducible_graphs(RBGraph& g) {

  std::list<SignedCharacter> realized_chars = realize_red_univ_and_univ_chars(g).first;
  remove_duplicate_species(g);
  std::list<SignedCharacter> tmp;

  if (logging::enabled) {
    std::cout << g << std::endl;
    std::cout << "[INFO] Removing any existing universal and red-universal characters, as well as duplicate species" << std::endl;
    std::cout << g << std::endl;
  }

  while (!is_empty(g)) {
     
    if (get_pending_species(g).size() == 1) {

      if (logging::enabled) 
        std::cout << "[INFO] Unique pending species found: realizing " << g[*get_pending_species(g).begin()].name << std::endl;

      tmp = realize_species(*get_pending_species(g).begin(), g).first;

    } else if (get_pending_species(g).size() == 2 && get_pending_and_minimal_p_active_species_with_unique_closest_neighbor(g) != 0 && get_inactive_chars(g).size() >= 4) {

      RBVertex species_to_realize = get_pending_and_minimal_p_active_species_with_unique_closest_neighbor(g);

      if (logging::enabled) {
        std::cout << "[INFO] There are two pending species and one of them is a minimal p-active species with a unique closest neighbor: realizing " << g[species_to_realize].name << std::endl;
      }

       tmp = realize_species(species_to_realize, g).first;

    } else if (get_minimal_p_active_species(g) != 0) {

      RBVertex species_to_realize = get_minimal_p_active_species(g);

      if (logging::enabled) 
        std::cout << "[INFO] Minimal p-active species found: realizing " << g[species_to_realize].name << std::endl;

      tmp = realize_species(species_to_realize, g).first;
    
    } else if (is_degenerate(g)) {

      if (logging::enabled) {
        std::cout << "[INFO] Graph is degenerate: starting realizing inactive characters: ";
        for (RBVertex c : get_inactive_chars(g))
          std::cout << g[c].name << " ";
        std::cout << std::endl;
      }

      for (RBVertex c : get_inactive_chars(g))
        tmp.splice(tmp.end(), realize_character({g[c].name, State::gain}, g).first);

    } else if (get_active_species(g).size() == 1) {

      RBVertex species_to_realize = *get_active_species(g).begin();

      if (logging::enabled) {
        std::cout << "[INFO] Found unique active species: realizing " << g[species_to_realize].name << std::endl;
      }

      tmp = realize_species(species_to_realize, g).first;

    } else if (get_quasi_active_species(g) != 0 && all_species_with_red_edges(g)) {

      RBVertex species_to_realize = get_quasi_active_species(g);

      if (logging::enabled) {
        std::cout << "[INFO] All species have red edges and there are quasi active species: realizing " << g[species_to_realize].name << std::endl;
      }

      tmp = realize_species(species_to_realize, g).first;

    } else {

      if (has_red_sigmagraph(g))
        std::cout << "[INFO] Red sigma graph generated" << std::endl;
      throw std::runtime_error("[ERROR] In ppp_maximal_reducible_graphs(): could not build the PPP");

    }

    if (logging::enabled) {
      std::cout << g << std::endl;
      std::cout << "[INFO] Removing any existing universal and red-universal characters, as well as duplicate species" << std::endl;
    }

    realized_chars.splice(realized_chars.end(), tmp);
    realized_chars.splice(realized_chars.end(), realize_red_univ_and_univ_chars(g).first);
    remove_duplicate_species(g);

    if (logging::enabled) {
      std::cout << g << std::endl;
    }

    if (!is_empty(g)) {
      RBGraphVector conn_compnts = connected_components(g);
      auto cc = conn_compnts.begin();
      auto cc_end = conn_compnts.end();
      for (; cc != cc_end; ++cc) {
        if (is_empty(*cc->get()))
          continue;
        RBGraph tmp_graph;
        copy_graph(*cc->get(), tmp_graph);
        if (logging::enabled) {
          std::cout << "[INFO] Recursive call ppp_maximal_reducible_graph() on the following graph: " << std::endl;
          std::cout << tmp_graph << std::endl;
        }
        tmp = ppp_maximal_reducible_graphs(*cc->get());
        for (RBVertex v : tmp_graph.m_vertices)
          remove_vertex(tmp_graph[v].name, g);
        realized_chars.splice(realized_chars.end(), tmp);
      }
    }
  }
  return realized_chars;
}


std::pair<std::list<SignedCharacter>, bool> realize_red_univ_and_univ_chars(RBGraph& g) {
  std::list<SignedCharacter> output;

  RBVertexIter v, v_end, next;
  std::tie(v, v_end) = vertices(g);

  std::list<SignedCharacter> lsc;
  for (next = v; v != v_end; v = next) {
    next++;
    if (is_red_universal(*v, g)) {
      // if v is red_universal
      // realize v-
      if (logging::enabled) {
        // verbosity enabled
        std::cout << "[INFO] found red-universal character to realize: " << g[*v].name << std::endl;
      }
      std::tie(lsc, std::ignore) = realize_character({g[*v].name, State::lose}, g);

      output.splice(output.cend(), lsc);

      // we have to loop again from the beginning, because if some isolate node has been deleted, the for loop is compromised
      std::tie(v, v_end) = vertices(g);
      next = v;
    } else if (is_universal(*v, g)) {
      // if v is universal
      // realize v+
      if (logging::enabled) {
        // verbosity enabled
        std::cout << "[INFO] Found universal character to realize: " << g[*v].name << std::endl;
      }

      std::list<SignedCharacter> lsc;

      std::tie(lsc, std::ignore) = realize_character({g[*v].name, State::gain}, g);           

      output.splice(output.cend(), lsc);

      // we have to loop again from the beginning, because if some isolate node has been deleted, the for loop is compromised
      std::tie(v, v_end) = vertices(g);
      next = v;
    }
  }

  if (!output.empty())
    return std::make_pair(output, true);
  else
    return std::make_pair(output, false);
}


RBVertex get_extension(const RBVertex& s, const RBGraph& gmax, const RBGraph& gmin) {
  std::cout << "*** get_extension ***" << std::endl;

  // ******** CASE 1 ********
  // check whether s already exists in gmin without any additional minimal characters. If it exists, then we have already found the extension of s.

  // find the characters of s
  std::set<std::string> s_chars;
  for (RBVertex v : get_adj_vertices(s, gmax)) 
    s_chars.insert(gmax[v].name);
  
  for (RBVertex v : gmin.m_vertices) {
    if (is_species(v, gmin)) {
      std::set<std::string> v_chars;
      for (RBVertex u : get_adj_vertices(v, gmin)) 
        v_chars.insert(gmin[u].name);
      
      if (v_chars == s_chars)
        return v;
    }
  }

  // ******** CASE 2 ********
  // find all the species such that (1) they include s and (2) they do not have additional maximal characters. These species will be possible candidates

  std::list<RBVertex> max_chars_of_gmin = maximal_characters(gmin);
  std::list<RBVertex> candidates; // it contains the possible candidates
  for (RBVertex v : gmin.m_vertices) { 
    if (is_species(v, gmin)) {

      // find the characters of v
      std::set<std::string> v_chars; 
      for (RBVertex u : get_adj_vertices(v, gmin)) 
        v_chars.insert(gmin[u].name);

      // v must include s
      if (std::includes(v_chars.begin(), v_chars.end(), 
                        s_chars.begin(), s_chars.end())) {
        std::cout << "includes " << gmin[v].name << std::endl;

        // v must not have more max chars than s
        bool more_max_chars = false;
        for (RBVertex max_char : max_chars_of_gmin) {
          if (!exists(gmax[s].name, gmax[max_char].name, gmax) && 
               exists(gmin[v].name, gmin[max_char].name, gmin)) {
            more_max_chars = true;
            break;
          }
        }

        if (!more_max_chars) { 
          std::cout << "has not more max chars than s " << gmin[v].name << std::endl;
          candidates.push_back(v);
        }

      }
    }
  }

  // if there is only a candidate, then return it
  if (candidates.size() == 1)
    return *candidates.begin();


// if we have more candidates, we have to choose the candidate X that satisfies
// the following property: (1) the realization of the characters of X and of
// all the minimal characters that overlap with the minimal characters of s 
// must not induce a red-sigma graph and (2) X must be minimal.

// we first sort the candidate list so that (2) is satisfied
sort_by_degree_desc(candidates, gmin);
candidates.reverse();
for (RBVertex candidate : candidates) {

  // this list will contain the minimal characters that overlap with the ones 
  // of s. We are going to build it by first inserting all the vertices of gmin 
  // and then reducing it progressively.
  std::set<RBVertex> overlapping_min_chars(gmin.m_vertices.begin(), 
                                           gmin.m_vertices.end());
  for (RBVertex u : gmin.m_vertices) 
    if (is_species(u, gmin))
      overlapping_min_chars.erase(u);

  std::list<RBVertex> candidate_chars = get_adj_vertices(candidate, gmin);
  for (RBVertex u : candidate_chars) 
    overlapping_min_chars.erase(u);

  for (RBVertex u : max_chars_of_gmin) 
    overlapping_min_chars.erase(u);

  for (RBVertex z : overlapping_min_chars) 
    for (RBVertex u : candidate_chars) 
      if (!overlaps_character(z, u, gmin)) 
        overlapping_min_chars.erase(z);

  // check for the red-sigma graph
  RBGraph g_copy;
  copy_graph(gmin, g_copy);
  realize_species(get_vertex(gmin[candidate].name, g_copy), g_copy);
  for (RBVertex u : overlapping_min_chars) 
    if (exists(gmin[u].name, g_copy))
      realize_species(get_vertex(gmin[u].name, g_copy), g_copy);
          
  if (!has_red_sigmagraph(g_copy)) {
    std::cout << "Candidate does not induce red sigmagraph" << std::endl;
    return candidate;
  } else {
    std::cout << "RED sigmagraph" << std::endl;
  }

}

return 0; // return null
}

std::list<RBVertex> get_sources(const RBGraph& gm) {
  std::list<RBVertex> sources = get_all_minimal_p_active_species(gm, true);
  if (sources.empty()) {
    std::list<RBVertex> pending_species = get_pending_species(gm);
    if (pending_species.size() == 1)
      sources.push_back(*pending_species.begin());
  }
  return sources;
}

bool is_2_solvable(std::list<RBVertex>& sources, const RBGraph& gm) {
  if (sources.size() != 2)
    return false;
  
  RBVertex source1 = *sources.begin();
  RBVertex source2 = *++sources.begin();
  if (get_adj_vertices(source1, gm).size() < 2 || 
      get_adj_vertices(source2, gm).size() < 2)
      return false;
  
  return true;  
}


std::list<RBVertex> closure(const RBVertex& v, const RBGraph& g) {
  
  // get the minimal characters as follow:
  // cmin = vertices(g) - cmax - species(g)
  std::list<RBVertex> cmax = maximal_characters(g);
  std::list<RBVertex> cmin(g.m_vertices.begin(), g.m_vertices.end());
  RBVertexIter b = cmin.begin(), e = cmin.end(), next;
  for (next = b; b != e; b = next) {
    ++next;
    if (is_species(*b, g))
      cmin.remove(*b);
    for (RBVertex u : cmax)
      if (g[*b].name == g[u].name) {
        cmin.remove(*b);
        break;
      }
  }

  // keep just the max chars of v
  b = cmax.begin(), e = cmax.end();
  for (next = b; b != e; b = next) {
    ++next;
    if (!exists(*b, v, g))
      cmax.remove(*b);
  }

  // keep just the min chars that are included in all max chars of v
  b = cmin.begin(), e = cmin.end();
  for (next = b; b != e; b = next) {
    ++next;
    for (RBVertex u : cmax) 
      if (!includes_characters(u, *b, g)) {
        cmin.remove(*b);
        break;
      }
  }

  return cmin;
}
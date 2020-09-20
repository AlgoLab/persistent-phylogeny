#include "functions.hpp"
#include <boost/graph/connected_components.hpp>
#include <boost/graph/depth_first_search.hpp>

//=============================================================================
// Auxiliary structs and classes

initial_state_visitor::initial_state_visitor()
    : m_safe_sources{}, m_sources{}, chain{}, source_v{}, last_v{} {}

initial_state_visitor::initial_state_visitor(std::list<HDVertex>& safe_sources,
                                             std::list<HDVertex>& sources)
    : m_safe_sources{&safe_sources},
      m_sources{&sources},
      chain{},
      source_v{},
      last_v{} {
  m_safe_sources->clear();
  m_sources->clear();
}

void initial_state_visitor::initialize_vertex(const HDVertex v,
                                              const HDGraph& hasse) const {}

void initial_state_visitor::start_vertex(const HDVertex v,
                                         const HDGraph& hasse) {
  if (logging::enabled) {
    // verbosity enabled
    std::cout << "DFS: start_vertex: [ ";

    for (const auto& kk : hasse[v].species) {
      std::cout << kk << " ";
    }

    std::cout << "]" << std::endl;
  }

  source_v = v;

  chain.clear();
}

void initial_state_visitor::discover_vertex(const HDVertex v,
                                            const HDGraph& hasse) {
  if (logging::enabled) {
    // verbosity enabled
    std::cout << "DFS: discover_vertex: [ ";

    for (const auto& kk : hasse[v].species) {
      std::cout << kk << " ";
    }

    std::cout << "]" << std::endl;
  }

  last_v = v;
}

void initial_state_visitor::examine_edge(const HDEdge e, const HDGraph& hasse) {
  HDVertex vs, vt;
  std::tie(vs, vt) = incident(e, hasse);

  if (logging::enabled) {
    // verbosity enabled
    std::cout << "DFS: examine_edge: [ ";

    for (const auto& kk : hasse[vs].species) {
      std::cout << kk << " ";
    }

    std::cout << "] -";

    auto jj = hasse[e].signedcharacters.cbegin();
    for (; jj != hasse[e].signedcharacters.cend(); ++jj) {
      std::cout << *jj;

      if (std::next(jj) != hasse[e].signedcharacters.cend()) std::cout << ",";
    }

    std::cout << "-> [ ";

    for (const auto& kk : hasse[vt].species) {
      std::cout << kk << " ";
    }

    std::cout << "]" << std::endl;
  }

  // add edge to the chain
  chain.push_back(e);
}

void initial_state_visitor::tree_edge(const HDEdge e,
                                      const HDGraph& hasse) const {
  if (logging::enabled) {
    // verbosity enabled
    HDVertex vs, vt;
    std::tie(vs, vt) = incident(e, hasse);

    std::cout << "DFS: tree_edge: [ ";

    for (const auto& kk : hasse[vs].species) {
      std::cout << kk << " ";
    }

    std::cout << "] -";

    auto jj = hasse[e].signedcharacters.cbegin();
    for (; jj != hasse[e].signedcharacters.cend(); ++jj) {
      std::cout << *jj;

      if (std::next(jj) != hasse[e].signedcharacters.cend()) std::cout << ",";
    }

    std::cout << "-> [ ";

    for (const auto& kk : hasse[vt].species) {
      std::cout << kk << " ";
    }

    std::cout << "]" << std::endl;
  }

  // ignore
}

void initial_state_visitor::back_edge(const HDEdge e,
                                      const HDGraph& hasse) const {
  if (logging::enabled) {
    // verbosity enabled
    HDVertex vs, vt;
    std::tie(vs, vt) = incident(e, hasse);

    std::cout << "DFS: back_edge: [ ";

    for (const auto& kk : hasse[vs].species) {
      std::cout << kk << " ";
    }

    std::cout << "] -";

    auto jj = hasse[e].signedcharacters.cbegin();
    for (; jj != hasse[e].signedcharacters.cend(); ++jj) {
      std::cout << *jj;

      if (std::next(jj) != hasse[e].signedcharacters.cend()) std::cout << ",";
    }

    std::cout << "-> [ ";

    for (const auto& kk : hasse[vt].species) {
      std::cout << kk << " ";
    }

    std::cout << "]" << std::endl;
  }

  // ignore
}

void initial_state_visitor::forward_or_cross_edge(const HDEdge e,
                                                  const HDGraph& hasse) {
  HDVertex vs, vt;
  std::tie(vs, vt) = incident(e, hasse);

  if (logging::enabled) {
    // verbosity enabled
    std::cout << "DFS: forward_or_cross_edge: [ ";

    for (const auto& kk : hasse[vs].species) {
      std::cout << kk << " ";
    }

    std::cout << "] -";

    auto jj = hasse[e].signedcharacters.cbegin();
    for (; jj != hasse[e].signedcharacters.cend(); ++jj) {
      std::cout << *jj;

      if (std::next(jj) != hasse[e].signedcharacters.cend()) std::cout << ",";
    }

    std::cout << "-> [ ";

    for (const auto& kk : hasse[vt].species) {
      std::cout << kk << " ";
    }

    std::cout << "]" << std::endl;
  }

  if (out_degree(vt, hasse) > 1) {
    // e is not the last edge in the chain (vt is not a sink in hasse),
    // ignore it and keep going
    return;
  }

  auto v_test = vt;

  while (out_degree(v_test, hasse) == 1) {
    // e is not the last edge in the chain (vt is not a sink in hasse),
    // but it may be needed for the chain to complete

    HDOutEdgeIter oe;
    std::tie(oe, std::ignore) = out_edges(v_test, hasse);

    // add outedge to the chain
    chain.push_back(*oe);

    // update v_test to current vt
    v_test = target(*oe, hasse);
  }

  perform_test(v_test, hasse);
}

void initial_state_visitor::finish_vertex(const HDVertex v,
                                          const HDGraph& hasse) {
  if (logging::enabled) {
    // verbosity enabled
    std::cout << "DFS: finish_vertex: [ ";

    for (const auto& kk : hasse[v].species) {
      std::cout << kk << " ";
    }

    std::cout << "]" << std::endl;
  }

  // build list of vertices in chain
  std::list<HDVertex> chain_v;
  for (const auto& e : chain) {
    chain_v.push_back(source(e, hasse));
  }

  bool v_in_chain =
      (std::find(chain_v.cbegin(), chain_v.cend(), v) != chain_v.cend());

  if (out_degree(v, hasse) > 0 || v_in_chain || last_v != v) {
    // v is not the last vertex in the chain
    // (which means the visit is backtracking)
    return;
  }

  perform_test(v, hasse);
}

void initial_state_visitor::perform_test(const HDVertex v,
                                         const HDGraph& hasse) {
  if (m_safe_sources == nullptr || m_sources == nullptr)
    // uninitialized sources lists
    return;

  // source_v holds the source vertex of the chain
  // chain holds the list of edges representing the chain

  // check if source_v is already in m_safe_sources or m_sources
  if ((!m_safe_sources->empty() && source_v == m_safe_sources->back()) ||
      (!m_sources->empty() && source_v == m_sources->back())) {
    if (logging::enabled) {
      // verbosity enabled
      std::cout << std::endl
                << "Chain detected, but its Source has already been processed"
                << std::endl
                << std::endl;
    }

    return;
  }

  // test if chain is a safe chain
  if (!safe_chain(v, hasse))
    // chain is not a safe chain
    return;

  if (!realize_source(source_v, hasse))
    // source_v is not realizable
    return;

  // test is source_v is a safe source (for test 1)
  if (safe_source_test1(hasse)) {
    // source_v is a safe source, return (don't add it to m_sources)
    m_safe_sources->push_back(source_v);

    if (exponential::enabled || interactive::enabled || nthsource::index > 0) {
      // exponential algorithm or user interaction enabled
      // or safe source selection index is not 0
      if (logging::enabled) {
        // verbosity enabled
        std::cout << std::endl
                  << "Source added to the list of safe sources" << std::endl
                  << std::endl;
      }

      return;
    }

    throw InitialState();
  }

  // test if the list of safe sources is empty
  if (!m_safe_sources->empty()) {
    // list of safe sources is not empty, return (don't add it to m_sources)
    if (logging::enabled) {
      // verbosity enabled
      std::cout << std::endl
                << "Test 2 and 3 wouldn't be feasible: "
                << "the list of safe sources is not empty" << std::endl
                << std::endl;
    }

    return;
  }

  if (logging::enabled) {
    // verbosity enabled
    std::cout << std::endl
              << "Source added to the list of sources" << std::endl;

    if (!chain.empty()) std::cout << std::endl;
  }

  m_sources->push_back(source_v);
}

bool initial_state_visitor::safe_chain(const HDVertex v, const HDGraph& hasse) {
  if (orig_g(hasse) == nullptr || orig_gm(hasse) == nullptr)
    // uninitialized graph properties
    return false;

  const auto& gm = *orig_gm(hasse);

  // chain holds the list of edges representing the chain

  // test if the chain is empty
  if (chain.empty()) {
    if (logging::enabled) {
      // verbosity enabled
      std::cout << std::endl << "Empty chain" << std::endl << std::endl;
    }

    return true;
  }

  std::list<SignedCharacter> lsc;

  for (const auto& c : hasse[source_v].characters) {
      lsc.push_back({c, State::gain});
  }

  for (const auto& e : chain) {
    for (const auto& sc : hasse[e].signedcharacters) {
      const auto check_sc_in_chain =
          std::find(hasse[v].characters.cbegin(), hasse[v].characters.cend(),
                    sc.character);

      if (check_sc_in_chain == hasse[v].characters.cend())
        // ignore wrong edge (other chain, old edge)
        break;

      const auto check_sc_in_lsc = std::find(lsc.cbegin(), lsc.cend(), sc);

      if (check_sc_in_lsc != lsc.cend())
        // remove previous signed character in lsc - it might be out of order
        lsc.erase(check_sc_in_lsc);

      lsc.push_back(sc);
    }
  }

  std::list<SignedCharacter> rsc;
  for(const auto sc : lsc) {
    if(is_active(get_vertex(sc.character,gm), gm))
      rsc.push_back(sc);
  }
  for(const auto sc : rsc)
    lsc.remove(sc);


  if (logging::enabled) {
    // verbosity enabled
    std::cout << std::endl << "Test chain: < ";

    for (const auto& kk : lsc) {
      std::cout << kk << " ";
    }

    std::cout << "> on a copy of graph Gm" << std::endl;
  }

  // copy gm to gm_test
  RBGraph gm_test;
  copy_graph(gm, gm_test);

  // test if lsc is a safe chain
  bool feasible;
  std::tie(std::ignore, feasible) = realize(lsc, gm_test);

  if (logging::enabled) {
    // verbosity enabled
    std::cout << std::endl
              << "Gm (copy) after the realization of the chain" << std::endl
              << "Adjacency lists:" << std::endl
              << gm_test << std::endl
              << std::endl;
  }

  if (!feasible) {
    if (logging::enabled) {
      // verbosity enabled
      std::cout << "Realization not feasible for Gm (copy)" << std::endl
                << std::endl;
    }

    return false;
  }

  // if the realization didn't induce a red Σ-graph, chain is a safe chain
  const auto output = !has_red_sigmagraph(gm_test);

  if (logging::enabled) {
    // verbosity enabled
    if (output)
      std::cout << "No red Σ-graph in Gm (copy)" << std::endl << std::endl;
    else
      std::cout << "Found red Σ-graph in Gm (copy)" << std::endl << std::endl;
  }

  return output;
}

bool initial_state_visitor::safe_source_test1(const HDGraph& hasse) {
  if (orig_g(hasse) == nullptr || orig_gm(hasse) == nullptr)
    // uninitialized graph properties
    return false;

  const auto& gm = *orig_gm(hasse);

  if (logging::enabled) {
    // verbosity enabled
    std::cout << std::endl << "Safe sources - test 1" << std::endl;
  }

  // search for a species s+ in GRB|CM∪A that consists of C(s) and is connected
  // to only inactive characters
  for (const auto& species_name : hasse[source_v].species) {
    const auto source_s = get_vertex(species_name, gm);
    // for each source species (s+) in source_v
    bool active = false;

    // check if s+ is connected to active characters
    RBOutEdgeIter e, e_end;
    std::tie(e, e_end) = out_edges(source_s, gm);
    for (; e != e_end; ++e) {
      // for each out egde from s+
      if (is_red(*e, gm)) {
        // s+ is connected to active characters; search for another species
        active = true;

        break;
      }
    }

    if (active)
      // s+ is connected to active characters
      continue;

    if (logging::enabled) {
      // verbosity enabled
      std::cout << "Source species: " << species_name << std::endl;
    }

    return true;
  }

  if (logging::enabled) {
    // verbosity enabled
    std::cout << "Safe sources - test 1 failed" << std::endl;
  }

  return false;
}

//=============================================================================
// Algorithm functions

std::list<HDVertex> initial_states(const HDGraph& hasse) {
  std::list<HDVertex> output;

  if (logging::enabled) {
    // verbosity enabled
    std::cout << "DFS visit on the Hasse diagram:" << std::endl << std::endl;
  }

  // the visitor continuosly modifies the sources variable (passed as reference)
  // in search of safe chains and sources. At the end of the visit, sources
  // holds the list of sources of the Hasse diagram.
  std::list<HDVertex> sources;
  initial_state_visitor vis(output, sources);

  std::map<HDVertex, size_t> i_map;
  for(auto v : boost::make_iterator_range(vertices(hasse)))
    i_map.emplace(v, i_map.size());
  
  
  auto ipmap = boost::make_assoc_property_map(i_map);
  
  std::vector<boost::default_color_type> c_map(num_vertices(hasse));
  auto cpmap = boost::make_iterator_property_map(c_map.begin(), ipmap);

  
  

  try {
    depth_first_search(hasse, 
                      boost::visitor(vis)
                      .vertex_index_map(ipmap)
                      .color_map(cpmap));
  } catch (const InitialState& e) {
  }

  if (logging::enabled) {
    // verbosity enabled
    std::cout << std::endl
              << "DFS visit on the Hasse diagram terminated" << std::endl
              << std::endl;
  }

  if (output.empty() && sources.size() == 1) {
    const auto source = sources.front();

    if (realize_source(source, hasse)) output.push_back(sources.front());
  } else if (output.empty() && sources.size() > 1) {
    if (logging::enabled) {
      // verbosity enabled
      std::cout << "Sources: < ";

      for (const auto& i : sources) {
        std::cout << "[ ";

        for (const auto& kk : hasse[i].species) {
          std::cout << kk << " ";
        }

        std::cout << "( ";

        for (const auto& kk : hasse[i].characters) {
          std::cout << kk << " ";
        }

        std::cout << ") ] ";
      }

      std::cout << ">" << std::endl << std::endl;
    }

    output = safe_source_test2(sources, hasse);

    if (output.empty()) output = safe_source_test3(sources, hasse);
  }

  if (logging::enabled) {
    // verbosity enabled
    std::cout << "Safe sources: < ";

    for (const auto& i : output) {
      std::cout << "[ ";

      for (const auto& kk : hasse[i].species) {
        std::cout << kk << " ";
      }

      std::cout << "( ";

      for (const auto& kk : hasse[i].characters) {
        std::cout << kk << " ";
      }

      std::cout << ") ] ";
    }

    std::cout << ">" << std::endl << std::endl;
  }

  return output;
}

std::list<HDVertex> safe_source_test2(const std::list<HDVertex>& sources,
                                      const HDGraph& hasse) {
  std::list<HDVertex> output;

  if (orig_g(hasse) == nullptr || orig_gm(hasse) == nullptr)
    // uninitialized graph properties
    return output;

  // const RBGraph& g = *orig_g(hasse);
  const RBGraph& gm = *orig_gm(hasse);

  if (logging::enabled) {
    // verbosity enabled
    std::cout << std::endl << "Safe sources - test 2" << std::endl;
  }

  // list of characters of GRB|CM∪A
  std::list<std::string> gm_c;
  RBVertexIter v, v_end;
  std::tie(v, v_end) = vertices(gm);
  for (; v != v_end; ++v) {
    if (!is_character(*v, gm)) continue;

    gm_c.push_back(gm[*v].name);
  }

  for (const auto& source : sources) {
    // list of characters of source
    const auto source_c = hasse[source].characters;

    // search for a species s+ in GRB|CM∪A that consists of C(s) and a set of
    // maximal characters, and is connected to only inactive characters
    std::tie(v, v_end) = vertices(gm);
    for (; v != v_end; ++v) {
      if (!is_species(*v, gm)) continue;

      // if s+ is in source it means that it was already tested in Test 1
      const auto search = std::find(hasse[source].species.cbegin(),
                                    hasse[source].species.cend(), gm[*v].name);

      if (search != hasse[source].species.cend())
        // s+ is in source, search for another species
        continue;

      std::list<std::string> maximal_c;
      size_t count_maximal = 0;
      bool active = false;

      RBOutEdgeIter e, e_end;
      std::tie(e, e_end) = out_edges(*v, gm);
      for (; e != e_end; ++e) {
        // for each out egde from s+
        if (is_red(*e, gm)) {
          // s+ is connected to active characters; search for another species
          active = true;

          break;
        } else {
          const auto vt = target(*e, gm);

          // search if vt is a maximal character in source_c
          auto search =
              std::find(source_c.cbegin(), source_c.cend(), gm[vt].name);

          if (search == source_c.cend()) {
            // vt is not a maximal character in source_c

            // search if vt is a maximal character in GRB|CM∪A
            search = std::find(gm_c.cbegin(), gm_c.cend(), gm[vt].name);

            if (search != gm_c.cend())
              // add vt to the set of maximal characters
              maximal_c.push_back(gm[vt].name);
          } else {
            count_maximal++;
          }
        }
      }

      if (count_maximal < source_c.size() || active)
        // s+ doesn't consist of C(s) or it's connected to active characters
        continue;

      if (maximal_c.size() == 0)
        // s+ doesn't have a set of other maximal characters
        continue;

      if (logging::enabled) {
        // verbosity enabled
        std::cout << "Source species (+ other maximal characters): "
                  << gm[*v].name << std::endl;
      }

      output.push_back(source);

      break;
    }

    if (output.empty()) continue;

    if (exponential::enabled || interactive::enabled || nthsource::index > 0) {
      // exponential algorithm or user interaction enabled
      // or safe source selection index is not 0
      if (logging::enabled) {
        // verbosity enabled
        std::cout << std::endl
                  << "Source added to the list of safe sources" << std::endl
                  << std::endl;
      }

      continue;
    }

    return output;
  }

  if (logging::enabled) {
    // verbosity enabled
    if (output.empty())
      std::cout << "Safe sources - test 2 failed" << std::endl;
  }

  return output;
}

std::list<HDVertex> safe_source_test3(const std::list<HDVertex>& sources,
                                      const HDGraph& hasse) {
  std::list<HDVertex> output;

  if (orig_g(hasse) == nullptr || orig_gm(hasse) == nullptr)
    // uninitialized graph properties
    return output;

  const auto& gm = *orig_gm(hasse);

  if (logging::enabled) {
    // verbosity enabled
    std::cout << std::endl << "Safe sources - test 3" << std::endl;
  }

  HDVertexIMap source_map;

  // make sure every source is connected to active characters
  for (const auto& source : sources) {
    bool source_active = true;

    // make sure every species s+ is connected to active characters
    for (const auto& species_name : hasse[source].species) {
      const auto source_s = get_vertex(species_name, gm);
      // for each source species (s+) in source
      size_t active_count = 0;

      // check if s+ is connected to active characters
      RBOutEdgeIter e, e_end;
      std::tie(e, e_end) = out_edges(source_s, gm);
      for (; e != e_end; ++e) {
        // for each out egde from s+
        if (!is_red(*e, gm)) continue;

        active_count++;
      }

      if (active_count == 0) {
        // s+ is not connected to active characters
        source_active = false;

        break;
      }

      if (source_map[source] > 0 && active_count >= source_map[source])
        continue;

      source_map[source] = active_count;
    }

    if (!source_active)
      // if s+ could not be found, the test fails
      return output;
  }

  size_t min_active_count = 0;
  std::list<HDVertex> maybe_output;

  for (const auto& pair : source_map) {
    const auto active_count = pair.second;

    if (min_active_count > 0 && active_count >= min_active_count) continue;

    min_active_count = active_count;
  }

  for (const auto& pair : source_map) {
    const auto source = pair.first;
    const auto active_count = pair.second;

    if (active_count > min_active_count) continue;

    maybe_output.push_back(source);
  }

  for (const auto& source : maybe_output) {
    if (logging::enabled) {
      // verbosity enabled
      std::cout << "Source (+ active characters): [ ";

      for (const auto& kk : hasse[source].species) {
        std::cout << kk << " ";
      }

      std::cout << "( ";

      for (const auto& kk : hasse[source].characters) {
        std::cout << kk << " ";
      }

      std::cout << ") ]" << std::endl;
    }

    output.push_back(source);

    if (exponential::enabled || interactive::enabled || nthsource::index > 0) {
      // exponential algorithm or user interaction enabled
      // or safe source selection index is not 0
      if (logging::enabled) {
        // verbosity enabled
        std::cout << std::endl
                  << "Source added to the list of safe sources" << std::endl
                  << std::endl;
      }

      continue;
    }

    return output;
  }

  if (logging::enabled) {
    // verbosity enabled
    if (output.empty())
      std::cout << "Safe sources - test 3 failed" << std::endl;
  }

  return output;
}

bool realize_source(const HDVertex source, const HDGraph& hasse) {
  if (orig_g(hasse) == nullptr || orig_gm(hasse) == nullptr)
    // uninitialized graph properties
    return false;

  const auto& gm = *orig_gm(hasse);

  if (logging::enabled) {
    // verbosity enabled
    std::cout << "Test source realization: [ ";

    for (const auto& kk : hasse[source].species) {
      std::cout << kk << " ";
    }

    std::cout << "( ";

    for (const auto& kk : hasse[source].characters) {
      std::cout << kk << " ";
    }

    std::cout << ") ] on a copy of graph G" << std::endl;
  }

  // copy g to g_test
  RBGraph gm_test;
  copy_graph(gm, gm_test);
  
  RBVertex s = get_vertex(hasse[source].species.front(), gm_test);
  auto acc = get_comp_active_characters(s, gm_test);

  for(const auto& elem : hasse[source].species) {
    for(const auto& ac : acc)
      add_edge(get_vertex(elem, gm_test), ac, gm_test);
  }
  

  // initialize the list of characters of source
  std::list<SignedCharacter> source_lsc;
  for (const auto& ci : hasse[source].characters) {
    if(is_inactive(get_vertex(ci, gm_test), gm_test)) 
      source_lsc.push_back({ci, State::gain});
  }

  bool feasible;
  std::tie(std::ignore, feasible) = realize(source_lsc, gm_test);

  if (logging::enabled) {
    // verbosity enabled
    std::cout << std::endl
              << "Gm (copy) after the realization of the source" << std::endl
              << "Adjacency lists:" << std::endl
              << gm_test << std::endl
              << std::endl;
  }

  if (!feasible) {
    if (logging::enabled) {
      // verbosity enabled
      std::cout << "Realization not feasible for Gm (copy)" << std::endl;
    }

    return false;
  }

  // if the realization didn't induce a red Σ-graph, source is a safe source
  const auto output = !has_red_sigmagraph(gm_test);

  if (logging::enabled) {
    // verbosity enabled
    if (output)
      std::cout << "No red Σ-graph in Gm (copy)" << std::endl;
    else
      std::cout << "Found red Σ-graph in Gm (copy)" << std::endl;
  }

  return output;
}

bool is_partial(const std::list<SignedCharacter>& reduction) {
  std::list<std::string> gained_c{};

  for (const auto& sc : reduction) {
    if (sc.state == State::gain) {
      gained_c.push_back(sc.character);

      continue;
    }

    const auto find_gained =
        std::find(gained_c.cbegin(), gained_c.cend(), sc.character);

    if (find_gained == gained_c.cend()) {
      return true;
    }
  }

  return false;
}

//=============================================================================
// Algorithm main functions

std::list<SignedCharacter> reduce(RBGraph& g) {
  std::list<SignedCharacter> output;

  if (logging::enabled) {
    // verbosity enabled
    
    std::cout << std::endl
              << "Working on the red-black graph G" << std::endl
              << "Adjacency lists:" << std::endl
              << g << std::endl
              << std::endl;
  }

  // cleanup graph from dead vertices
  // TODO: check if this is needed (realize already does this?)
  remove_singletons(g);

  if (is_empty(g)) {
    // if graph is empty
    // return the empty sequence
    if (logging::enabled) {
      // verbosity enabled
      std::cout << "G empty" << std::endl << std::endl;
    }

    // return < >
    return output;
  }

  if (logging::enabled) {
    // verbosity enabled
    std::cout << "G not empty" << std::endl;
  }

  RBGraphVector components;
  RBVertexIMap i_map, c_map;
  RBVertexIAssocMap i_assocmap(i_map), c_assocmap(c_map);

  // fill the vertex index map i_assocmap
  RBVertexIter v, v_end;
  std::tie(v, v_end) = vertices(g);
  for (size_t index = 0; v != v_end; ++v, ++index) {
    boost::put(i_assocmap, *v, index);
  }

  // get number of components and the components map
  const size_t c_count = boost::connected_components(
      g, c_assocmap, boost::vertex_index_map(i_assocmap));

  // realize free characters in the graph
  // TODO: check if this is needed (realize already does this?)
  std::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    // for each vertex
    if (is_red_universal(*v, g)) { // TODO attenzione (cambiato definizione di red-universal: prima un carattere era red universal se era attivo e connesso a tutte le specie della sua componente. Ora, invece, è coerente con il paper: un carattere è red universal quando è attivo ed è connesso a tutte le specie del grafo. Quindi attenzione: modificare questa istruzione if se si intende usare questa funzione per come era stata progettata prima.)

      // if v is free
      // realize v-
      // return < v-, reduce(g) >
      if (logging::enabled) {
        // verbosity enabled
        std::cout << "G free character " << g[*v].name << std::endl;
      }

      std::list<SignedCharacter> lsc;
      std::tie(lsc, std::ignore) = realize_character({g[*v].name, State::lose}, g);

      output.splice(output.cend(), lsc);
      output.splice(output.cend(), reduce(g));

      // return < v-, reduce(g) >
      return output;
    }
  }

  if (logging::enabled) {
    // verbosity enabled
    std::cout << "G no free characters" << std::endl;
  }

  // realize universal characters in the graph
  // TODO: check if this is needed (realize already does this?)
  std::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    // for each vertex
    if (is_universal(*v, g)) { // TODO attenzione (cambiato definizione di universal: prima un carattere era universal se era inattivo e connesso a tutte le specie della sua componente. Ora, invece, è coerente con il paper: un carattere è universal quando è inattivo ed è connesso a tutte le specie del grafo. Quindi attenzione: modificare questa istruzione if se si intende usare questa funzione per come era stata progettata prima.)
      // if v is universal
      // realize v+
      // return < v+, reduce(g) >
      if (logging::enabled) {
        // verbosity enabled
        std::cout << "G universal character " << g[*v].name << std::endl;
      }

      std::list<SignedCharacter> lsc;
      std::tie(lsc, std::ignore) = realize_character({g[*v].name, State::gain}, g);

      output.splice(output.cend(), lsc);
      output.splice(output.cend(), reduce(g));

      // return < v+, reduce(g) >
      return output;
    }
  }

  if (logging::enabled) {
    // verbosity enabled
    std::cout << "G no universal characters" << std::endl;
  }

  if (c_count > 1) {
    components = connected_components(g, c_map, c_count);
    // if graph is not connected
    // build subgraphs (connected components) g1, g2, etc.
    // return < reduce(g1), reduce(g2), ... >
    for (const auto& component : components) {
      output.splice(output.cend(), reduce(*component.get()));
    }
    // return < reduce(g1), reduce(g2), ... >
    return output;
  }
  else if(logging::enabled) {
    // verbosity enabled
    std::cout << "G connected" << std::endl;
  }

  if (logging::enabled) {
    // verbosity enabled
    std::cout << std::endl;
  }

  // gm = Grb|Cm∪A, maximal reducible graph of g (Grb)
  const auto gm = maximal_reducible_graph(g, true);

  if (logging::enabled) {
    // verbosity enabled
    std::cout << std::endl
              << "Subgraph Gm of G induced by the maximal characters Cm"
              << std::endl
              << "Adjacency lists:" << std::endl
              << gm << std::endl
              << std::endl;
  }

  if(logging::enabled) {
    auto ac = get_active_characters(gm);
    if(ac.size() <= 0)
      std::cout << "No active characters"
                << std::endl;
    else { 
      std::cout << "Active characters: ";
      for(RBVertex elem : ac)
        std::cout << g[elem].name << " ";
      std::cout << std::endl;
    }
  } 
  

  // p = Hasse diagram for gm (Grb|Cm∪A)
  HDGraph p;
  hasse_diagram(p, g, gm, components, c_map);

  if (logging::enabled) {
    // verbosity enabled
    std::cout << "Hasse diagram for the subgraph Gm" << std::endl
              << "Adjacency lists:" << std::endl
              << p << std::endl
              << std::endl;
  }

  // s = initial states
  std::list<HDVertex> s = initial_states(p);

  if (s.empty())
    // p has no safe source
    throw NoReduction();

  HDVertex source = 0;
  std::list<SignedCharacter> sc;

  // exponential safe source selection
  if (exponential::enabled) {
    // exponential algorithm enabled
    std::list<std::list<SignedCharacter>> sources_output;

    for (const auto& source : s) {
      // for each safe source in s
      RBGraph g_test;
      copy_graph(g, g_test);

      if (logging::enabled) {
        // verbosity enabled
        std::cout << "Current safe source: [ ";

        for (const auto& kk : p[source].species) {
          std::cout << kk << " ";
        }

        std::cout << "( ";

        for (const auto& kk : p[source].characters) {
          std::cout << kk << " ";
        }

        std::cout << ") ]" << std::endl << std::endl;
      }

      // realize the characters of the safe source
      sc.clear();

      for (const auto& ci : p[source].characters) {
        sc.push_back({ci, State::gain});
      }

      if (logging::enabled) {
        // verbosity enabled
        std::cout << "Realize the characters < ";

        for (const auto& kk : sc) {
          std::cout << kk << " ";
        }

        std::cout << "> in G" << std::endl;
      }

      std::tie(sc, std::ignore) = realize(sc, g_test);

      try {
        std::list<SignedCharacter> rest = reduce(g_test);

        if (logging::enabled) {
          // verbosity enabled
          std::cout << "Ok for safe source [ ";

          for (const auto& kk : p[source].species) {
            std::cout << kk << " ";
          }

          std::cout << "( ";

          for (const auto& kk : p[source].characters) {
            std::cout << kk << " ";
          }

          std::cout << ") ]" << std::endl << std::endl;
        }

        // append the recursive call to the current source's output
        sc.splice(sc.end(), rest);
        sources_output.push_back(sc);
      } catch (const NoReduction& e) {
        if (logging::enabled) {
          // verbosity enabled
          std::cout << "No for safe source [ ";

          for (const auto& kk : p[source].species) {
            std::cout << kk << " ";
          }

          std::cout << "( ";

          for (const auto& kk : p[source].characters) {
            std::cout << kk << " ";
          }

          std::cout << ") ]" << std::endl << std::endl;
        }
      }
    }

    if (sources_output.empty())
      // no realization induces a successful reduction
      throw NoReduction();

    if (logging::enabled) {
      // verbosity enabled
      std::cout << "Reductions: [" << std::endl;

      for (const auto& lkk : sources_output) {
        if (is_partial(lkk))
          std::cout << "  Partial: ";
        else
          std::cout << "  Complete: ";

        std::cout << "< ";

        for (const auto& kk : lkk) {
          std::cout << kk << " ";
        }

        std::cout << ">" << std::endl;
      }

      std::cout << "]" << std::endl << std::endl;
    }

    return sources_output.front();
  }
  // user-input-driven safe source selection
  else if (s.size() > 1 && interactive::enabled) {
    // user interaction enabled
    size_t choice = 0;

    if (!logging::enabled) {
      std::cout << std::endl << std::endl;
    }

    std::cout << "========================================"
              << "========================================" << std::endl
              << std::endl
              << "List of available source indexes to choose from:"
              << std::endl;

    size_t index = 0;
    for (const auto& source : s) {
      std::cout << "  - " << index << ": [ ";

      for (const auto& kk : p[source].species) {
        std::cout << kk << " ";
      }

      std::cout << "( ";

      for (const auto& kk : p[source].characters) {
        std::cout << kk << " ";
      }

      std::cout << ") ]" << std::endl;

      index++;
    }

    std::cout << std::endl;

    // get input
    std::string input;
    std::cout << "Choose a source: ";

    while (std::getline(std::cin, input)) {
      // if (input == "help" || input == "h") {
      //   // print help message
      // }

      // parse input as a number
      std::stringstream istream(input);
      if (istream >> choice) {
        // choice is a valid number
        if (choice < s.size()) {
          // choice is a valid safe source index
          // set the source
          source = *std::next(s.cbegin(), choice);

          std::cout << "Source [ ";

          for (const auto& kk : p[source].species) {
            std::cout << kk << " ";
          }

          std::cout << "( ";

          for (const auto& kk : p[source].characters) {
            std::cout << kk << " ";
          }

          std::cout << ") ] selected" << std::endl << std::endl;
          // exit the loop
          break;
        }
      }

      std::cout << "Error: invalid input."
                // << std::endl
                // << "Try 'help' or 'h' for more information."
                << std::endl
                << std::endl
                << "Choose a source: ";
    }

    sc.clear();

    for (const auto& ci : p[source].characters) {
      sc.push_back({ci, State::gain});
    }

    if (logging::enabled) {
      // verbosity enabled
      std::cout << "========================================"
                << "========================================" << std::endl
                << std::endl;
    }
  } else if (s.size() > 1 && nthsource::index > 0) {
    if (nthsource::index < s.size())
      source = *std::next(s.cbegin(), nthsource::index);
    else
      source = s.back();

    if (logging::enabled) {
      // verbosity enabled
      std::cout << "Source [ ";

      for (const auto& kk : p[source].species) {
        std::cout << kk << " ";
      }

      std::cout << "( ";

      for (const auto& kk : p[source].characters) {
        std::cout << kk << " ";
      }

      std::cout << ") ] selected " << std::endl << std::endl;
    }
  }
  // standard safe source selection (the first one found)
  else {
    source = s.front();
  }

  sc.clear();

  for (const auto& ci : p[source].characters) {
    sc.push_back({ci, State::gain});
  }

  if (logging::enabled) {
    // verbosity enabled
    std::cout << "Realize the characters < ";

    for (const auto& kk : sc) {
      std::cout << kk << " ";
    }

    std::cout << "> in G" << std::endl;
  }

  // realize the characters of the safe source
  std::tie(sc, std::ignore) = realize(sc, g);

  // append the list of realized characters and the recursive call to the
  // output in constant time (std::list::splice simply moves pointers around
  // instead of copying the data)
  output.splice(output.cend(), sc);
  output.splice(output.cend(), reduce(g));

  // return < sc, reduce(g) >
  return output;
}

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
      std::cout << "Realizing " << sc;
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
      std::cout << std::endl;
    }
  } else if (sc.state == State::lose && is_active(cv, g)) {
    // c- and c is active
    if (logging::enabled) {
      // verbosity enabled
      std::cout << "Realizing " << sc << std::endl;
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
          std::cout << "Could not realize " << sc << std::endl;
      }
      return std::make_pair(output, false);
    }
  } else {
    if (logging::enabled) {
      // verbosity enabled
      std::cout << "Could not realize " << sc << std::endl;
    }

    // this should never happen during the algorithm, but it is handled just in
    // case something breaks (or user input happens)
    return std::make_pair(output, false);
  }

  output.push_back(sc);

  // delete all isolated vertices
  remove_singletons(g);

  /*
  // fill vertex index map
  std::tie(v, v_end) = vertices(g);
  for (size_t index = 0; v != v_end; ++v, ++index) {
    boost::put(i_assocmap, *v, index);
  }

  // build the components map
  boost::connected_components(g, c_assocmap,
                              boost::vertex_index_map(i_assocmap));

  // realize all red_universal characters that came up after realizing sc
  std::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    // for each vertex
    if (is_red_universal(*v, g, c_map)) { // TODO attenzione (cambiato definizione di red-universal: prima un carattere era red universal se era attivo e connesso a tutte le specie della sua componente. Ora, invece, è coerente con il paper: un carattere è red universal quando è attivo ed è connesso a tutte le specie del grafo. Quindi attenzione: modificare questa istruzione if se si intende usare questa funzione per come era stata progettata prima.)
      // if v is red_universal
      // realize v-
      if (logging::enabled) {
        // verbosity enabled
        std::cout << "G red-universal character " << g[*v].name << std::endl;
      }

      std::list<SignedCharacter> lsc;
      std::tie(lsc, std::ignore) = realize({g[*v].name, State::lose}, g);

      output.splice(output.cend(), lsc);

      return std::make_pair(output, true);
    }
  }

  // realize all universal characters that came up after realizing sc
  std::tie(v, v_end) = vertices(g);
  for (; v != v_end; ++v) {
    // for each vertex
    if (is_universal(*v, g, c_map)) { // TODO attenzione (cambiato definizione di universal: prima un carattere era universal se era inattivo e connesso a tutte le specie della sua componente. Ora, invece, è coerente con il paper: un carattere è universal quando è inattivo ed è connesso a tutte le specie del grafo. Quindi attenzione: modificare questa istruzione if se si intende usare questa funzione per come era stata progettata prima.)
      // if v is universal
      // realize v+
      if (logging::enabled) {
        // verbosity enabled
        std::cout << "G universal character " << g[*v].name << std::endl;
      }

      std::list<SignedCharacter> lsc;
      std::tie(lsc, std::ignore) = realize({g[*v].name, State::gain}, g);

      output.splice(output.cend(), lsc);

      return std::make_pair(output, true);
    }
  }*/
  return std::make_pair(output, true);
}

std::pair<std::list<SignedCharacter>, bool> realize_species(const RBVertex v,
                                                    RBGraph& g) {
  std::list<SignedCharacter> lsc;

  if (!is_species(v, g)) 
    return std::make_pair(lsc, false);

  // build the list of inactive characters adjacent to v (species)
  std::list<RBVertex> adjacent_chars = get_adj_character_map(g)[v];
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
  for (const auto& i : lsc) {
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

/*
void realize_character(RBVertex& c, RBGraph& g) {
  if (is_active(c, g))
    realize({g[c].name, State::lose}, g);
  else
    realize({g[c].name, State::gain}, g);
  
  if (!exists(c, g))
    throw std::runtime_error("[ERROR] In realize_character(): input vertex does not exist");
  if (is_character(c, g)) {
    if (is_inactive(c, g)) {
      auto map = get_adjacent_species_map(g);
      std::list<RBVertex> adj_spec_to_c = map[c];
      // adj_spec_to_c contains the species adjacent to c

      std::list<std::string> species_comp_of_c = get_comp_vertex(c, g);
      // species_comp_of_c contains the names of the species in the component to which also c belongs

      // for every name of species s in the same component of c, if there already exists an edge from s to c, ignore it, otherwise add a red edge from s to c
      for (std::string s : species_comp_of_c) 
        if (!exists(g[c].name, s, g)) 
          add_edge(g[c].name, s, Color::red, g);      
      
      // remove all the black edges from c to its connected species
      for (RBVertex v : adj_spec_to_c)
        remove_edge(g[c].name, g[v].name, g);
    } else {
      // c is active. We have to check if c is connected to all the species of its connected component, otherwise it cannot be realized

      std::list<std::string> species_comp_of_c = get_comp_vertex(c, g);
      std::list<RBVertex> species_adj_to_c = get_adjacent_species_map(g)[c];

      bool connected = true;
      bool found;
      for (std::string s_comp : species_comp_of_c) {
        found = false;
        for (RBVertex v : species_adj_to_c)
          if (g[v].name == s_comp) {
            found = true;
            break;
          }
        if (!found) {
          connected = false;
          break;
        }
      }

      if (connected)
        clear_vertex(c, g); //< remove its red edges
    }

    remove_singletons(g);
  } 
}

void realize_species(RBVertex& s, RBGraph& g) {
  std::list<RBVertex> adjacent_chars = get_adjacent_characters_map(g)[s];
  
  // realize the inactive characters adjacent to species s
  for (RBVertex c : adjacent_chars)
    if (is_inactive(c, g))
      realize_character(c, g);
}*/

void order_by_degree(std::list<RBVertex>& list_to_order, const RBGraph& g) {
  // constructing a list of pairs <RBVertex, int>, where the first element is a vertex and the second element is the degree of the vertex
  std::list<std::pair<RBVertex, int>> list_of_pairs;
  
  for (RBVertex v : list_to_order)
    list_of_pairs.push_back(std::make_pair(v, out_degree(v, g)));
  
  auto comparator = [](const std::pair<RBVertex, int> &a, const std::pair<RBVertex, int> &b) { 
    return  a.second > b.second; 
  };

  list_of_pairs.sort(comparator);

  list_to_order.clear();

  for (std::pair<RBVertex, int> pair : list_of_pairs)
    list_to_order.push_back(pair.first);
}

RBVertex get_minimal_p_active_species(const RBGraph& g) {

  std::list<RBVertex> active_species = get_active_species(g);
  order_by_degree(active_species, g);

  RBVertex p_active_candidate = 0;
  bool found = false;
  int num_inctv_chars_v, num_inctv_chars_u;
  for (RBVertex v : active_species) {
    // for every active species v in the ordered list of active species
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
              p_active_candidate = v;
              found = true;
              break;
            }
          }
        }
      }
      if (found) break;
    }
    if (found) break;
  }
  return p_active_candidate;
}

bool is_quasi_active(const RBVertex& s, const RBGraph& g) {
  if (!is_species(s, g)) return false;

  if (is_active(s, g)) return true;

  // then s is a species with some red incoming edges
  // then we have to check whether its realization generates a red-sigmagraph

  RBGraph g_copy;
  copy_graph(g, g_copy);
  realize_species(get_vertex(g[s].name, g_copy), g_copy);
  if (has_red_sigmagraph(g_copy))
    return false;
  else
    return true;
}
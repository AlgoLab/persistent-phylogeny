/** 
 * 
 * @file rbgraph.hpp
 * 
 * @author Simone Paolo Mottadelli
 * 
 */

#ifndef RBGRAPH_HPP
#define RBGRAPH_HPP

#include <boost/graph/adjacency_list.hpp>
#include <iostream>
#include "globals.hpp"

//=============================================================================
// Forward declaration for typedefs


/**
  Red-black graph traits
*/
typedef boost::adjacency_list_traits<boost::listS,       // OutEdgeList
                                     boost::listS,       // VertexList
                                     boost::undirectedS  // Directed
                                     >
    RBTraits;


/**
  Map of strings and vertices (red-black graph)
*/
typedef std::map<std::string, RBTraits::vertex_descriptor> RBVertexNameMap;

//=============================================================================
// Data structures


/**
  Scoped enumeration type whose underlying size is 1 byte, used for edge color.

  Color is used to label edges in a red-black graph.
*/
enum class Color : bool {
  black,  ///< The character incident on the labeled edge is inactive
  red     ///< The character incident on the labeled edge is active
};


/**
  Scoped enumeration type whose underlying size is 1 byte, used for vertex type.

  Type is used to label vertices in a red-black graph.
*/
enum class Type : bool {
  species,   ///< The labeled vertex is a species
  character  ///< The labeled vertex is a character
};

//=============================================================================
// Bundled properties


/**
  @brief Struct used to represent the properties of an edge (red-black graph)

  Each character c ∈ C is incident only on black edges (in this case c is
  inactive), or it is incident only on red edges (in this case c is active).
*/
struct RBEdgeProperties {
  Color color{};  ///< Edge color (Red or Black)
};


/**
  @brief Struct used to represent the properties of a vertex (red-black graph)

  A red-black graph on a set S of species and a set C of characters, is a
  bipartite graph whose vertex set is S ∪ C.
*/
struct RBVertexProperties {
  std::string name{};  ///< Vertex name
  Type type{};         ///< Vertex type (Character or Species)
};


/**
  @brief Struct used to represent the properties of a red-black graph
*/
struct RBGraphProperties {
  size_t num_species{};     ///< Number of species in the graph
  size_t num_characters{};  ///< Number of characters in the graph

  RBVertexNameMap vertex_map{};  ///< Map for vertex names and vertices in the
                                 ///< graph
};

//=============================================================================
// Typedefs used for readabily

// Graph

/**
  Red-black graph
*/
typedef boost::adjacency_list<boost::listS,        // OutEdgeList
                              boost::listS,        // VertexList
                              boost::undirectedS,  // Directed
                              RBVertexProperties,  // VertexProperties
                              RBEdgeProperties,    // EdgeProperties
                              RBGraphProperties    // GraphProperties
                              >
    RBGraph;

// Descriptors


/**
  Edge of a red-black graph
*/
typedef boost::graph_traits<RBGraph>::edge_descriptor RBEdge;


/**
  Vertex of a red-black graph
*/
typedef boost::graph_traits<RBGraph>::vertex_descriptor RBVertex;

// Iterators


/**
  Iterator of vertices (red-black graph)
*/
typedef boost::graph_traits<RBGraph>::vertex_iterator RBVertexIter;


/**
  Iterator of outgoing edges (red-black graph)
*/
typedef boost::graph_traits<RBGraph>::out_edge_iterator RBOutEdgeIter;

// Size types


/**
  Size type of vertices (red-black graph)
*/
typedef boost::graph_traits<RBGraph>::vertices_size_type RBVertexSize;

// Maps


/**
  Map of vertex indexes (red-black graph)
*/
typedef std::map<RBVertex, RBVertexSize> RBVertexIMap;


/**
  Associative property map of vertex indexes (red-black graph)
*/
typedef boost::associative_property_map<RBVertexIMap> RBVertexIAssocMap;


/**
  Map of vertices (red-black graph)
*/
typedef std::map<RBVertex, RBVertex> RBVertexMap;


/**
  Associative property map of vertices (red-black graph)
*/
typedef boost::associative_property_map<RBVertexMap> RBVertexAssocMap;

// Containers


/**
  Vector of unique pointers to red-black graphs
*/
typedef std::vector<std::unique_ptr<RBGraph>> RBGraphVector;


//=============================================================================
// Auxiliary structs and classes


/**
  @brief Functor used in remove_vertex_if.
*/
struct if_singleton {
  /**
    @brief Overloading of operator() for if_singleton .

    @param[in] v Vertex
    @param[in] g Red-black graph

    @return True if \e v is a singleton in \e g
  */
  inline bool operator()(const RBVertex& v, const RBGraph& g) const {
    /*if (logging::enabled && out_degree(v, g) == 0)
      std::cout << "[INFO] Found singleton vertex to remove: " << g[v].name << std::endl;*/
    return (out_degree(v, g) == 0);
  }
};


/**
  @brief Functor used in remove_vertex_if .
*/
struct if_not_maximal {
  /**
    @brief Functor constructor

    @param[in] cm Maximal characters
  */
  if_not_maximal(const std::list<RBVertex>& cm) : m_cm{&cm} {};

  /**
    @brief Overloading of operator() for if_not_maximal .

    @param[in] v Vertex
    @param[in] g Red-black graph

    @return True if \e v is not maximal character of \e g
  */
  inline bool operator()(const RBVertex v, const RBGraph& g) const {
    if (m_cm == nullptr) return false;

    /*if (logging::enabled && std::find(m_cm->cbegin(), m_cm->cend(), v) != m_cm->cend())
      std::cout << "[INFO] Found maximal character to remove: " << g[v].name << std::endl;*/

    return (std::find(m_cm->cbegin(), m_cm->cend(), v) == m_cm->cend());
  }

 private:
  const std::list<RBVertex>* const m_cm{};
};


//=============================================================================
// Boost functions (overloading)


/**
  @brief Remove all vertices and edges from \e g .

  @param[in] g Red-black graph
*/
void clear(RBGraph& g);


/**
  @brief Remove the edge with source \e s and target \e t from \e g .

  @param[in]     s Source vertex
  @param[in]     t Target vertex
  @param[in,out] g Red-black graph

*/
void remove_edge(const RBVertex& s, const RBVertex& t, RBGraph& g);


/**
  @brief Remove the edge with source \e s and target \e t from \e g .

  @param[in]     s Source vertex name
  @param[in]     t Target vertex name
  @param[in,out] g Red-black graph

*/
void remove_edge(const std::string& s, const std::string& t, RBGraph& g);


/**
  @brief Remove \e e from \e g .

  @param[in]     e Edge
  @param[in,out] g Red-black graph

  @return true if the edge has been removed, false if the edge did not exist
*/
inline void remove_edge(const RBEdge& e, RBGraph& g) {
  remove_edge(e.m_source, e.m_target, g);
}


/**
  @brief Remove \e v from \e g .

  @param[in]     v Vertex
  @param[in,out] g Red-black graph
*/
void remove_vertex(const RBVertex& v, RBGraph& g);


/**
  @brief Remove vertex \e name from \e g .

  @param[in]     name Vertex name
  @param[in,out] g    Red-black graph
*/
void remove_vertex(const std::string& name, RBGraph& g);


/**
  @brief Add the vertex with \e name and \e type to \e g .

  @param[in]     name Name
  @param[in]     type Type
  @param[in,out] g    Red-black graph

  @return Vertex descriptor for the new vertex
*/
RBVertex add_vertex(const std::string& name, const Type type, RBGraph& g);


/**
  @brief Add the vertex (species) with \e name to \e g .

  @param[in]     name Name
  @param[in,out] g    Red-black graph

  @return Vertex descriptor for the new species
*/
inline RBVertex add_species(const std::string& name, RBGraph& g) {
  return add_vertex(name, Type::species, g);
}


/**
  @brief Add teh vertex (character) with \e name to \e g .

  @param[in]     name Name
  @param[in,out] g    Red-black graph

  @return Vertex descriptor for the new character
*/
inline RBVertex add_character(const std::string& name, RBGraph& g) {
  return add_vertex(name, Type::character, g);
}


/**
  @brief Add the edge between \e u and \e v with \e color to \e g .

  If the edge is already in the graph then a duplicate will not be
  added and the bool flag will be false.
  When the flag is false, the returned edge descriptor points to the
  already existing edge.

  @param[in]     u     Source Vertex
  @param[in]     v     Target Vertex
  @param[in]     color Color
  @param[in,out] g     Red-black graph

  @return Edge descriptor for the new edge.
*/
std::pair<RBEdge, bool> add_edge(const RBVertex& u, const RBVertex& v,
                                 const Color color, RBGraph& g);


/**
  @brief Add edge between \e u and \e v with \e color to \e g .

  If the edge is already in the graph then a duplicate will not be
  added and the bool flag will be false.
  When the flag is false, the returned edge descriptor points to the
  already existing edge.

  @param[in]     u     Source Vertex name
  @param[in]     v     Target Vertex name
  @param[in]     color Color
  @param[in,out] g     Red-black graph

  @return Edge descriptor for the new edge.
*/
std::pair<RBEdge, bool> add_edge(const std::string& source, const std::string& target, Color color, RBGraph& g);


/**
  @brief Add a black edge between \e u and \e v to \e g .

  If the edge is already in the graph then a duplicate will not be
  added and the bool flag will be false.
  When the flag is false, the returned edge descriptor points to the
  already existing edge.

  @param[in]     u Source Vertex
  @param[in]     v Target Vertex
  @param[in,out] g Red-black graph

  @return Edge descriptor for the new edge.
*/
inline std::pair<RBEdge, bool> add_edge(const RBVertex& u, const RBVertex& v,
                                        RBGraph& g) {
  return add_edge(u, v, Color::black, g);
}


//=============================================================================
// General functions


/**
  @brief Return the number of species in \e g .

  @param[in] g Red-black graph

  @return Reference to the number of species in \e g
*/
inline size_t& num_species(RBGraph& g) {
  return g[boost::graph_bundle].num_species;
}


/**
  @brief Return the number of species (const) in \e g .

  @param[in] g Red-black graph

  @return Constant number of species in \e g
*/
inline const size_t& num_species(const RBGraph& g) {
  return g[boost::graph_bundle].num_species;
}


/**
  @brief Return the number of characters in \e g .

  @param[in] g Red-black graph

  @return Reference to the number of characters in \e g
*/
inline size_t& num_characters(RBGraph& g) {
  return g[boost::graph_bundle].num_characters;
}


/**
  @brief Return the number of characters (const) in \e g .

  @param[in] g Red-black graph

  @return Constant number of characters in \e g
*/
inline const size_t num_characters(const RBGraph& g) {
  return g[boost::graph_bundle].num_characters;
}


/**
  @brief Return the map in \e g .
  The map M is such that M[key] = vertex object in \e g , where key is the string name of the vertex.

  @param[in] g Red-black graph

  @return Reference to the map in \e g
*/
inline RBVertexNameMap& vertex_map(RBGraph& g) {
  return g[boost::graph_bundle].vertex_map;
}


/**
  @brief Return the map (const) in \e g .

  @param[in] g Red-black graph

  @return Constant reference to the map in \e g
*/
inline const RBVertexNameMap& vertex_map(const RBGraph& g) {
  return g[boost::graph_bundle].vertex_map;
}


/**
  @brief Remove \e v from \e g if it satisfies \e predicate .

  @param[in]     v         Vertex
  @param[in]     predicate Predicate
  @param[in,out] g         Red-black graph
*/
template <typename Predicate>
void remove_vertex_if(const RBVertex& v, Predicate predicate, RBGraph& g) {
  if (predicate(v, g)) {
    // vertex satisfies the predicate
    clear_vertex(v, g);
    remove_vertex(v, g);
  }
}


/**
  @brief Return the edge descriptor of the edge with \e source and \e target as vertices in \e g .

  @param[in] source Source vertex name
  @param[in] target Target vertex name
  @param[in] g    Red-black graph

  @return Edge
*/
RBEdge get_edge(const RBVertex &source, const RBVertex &target, const RBGraph &g);


/**
  @brief Return a reference to the vertex descriptor of the vertex \e name in \e g .
 
  @param[in] name Vertex name
  @param[in] g    Red-black graph

  @return Costant reference to the vertex
*/
const RBVertex& get_vertex(const std::string& name, const RBGraph& g);


/**
  @brief Return true if an edge with \e source and \e target exists in \e g .

  @param[in] source Source vertex
  @param[in] target Target vertex
  @param[in] g    Red-black graph

  @return bool
*/
bool exists(const RBVertex &source, const RBVertex &target, const RBGraph &g);


/**
  @brief Return true if an edge with \e source and \e target exists in \e g .

  @param[in] source Source vertex name
  @param[in] target Target vertex name
  @param[in] g    Red-black graph

  @return bool
*/
bool exists(const std::string &source, const std::string &target, const RBGraph &g);


/**
  @brief Return true if \e v exists in \e g .

  @param[in] v    Vertex
  @param[in] g    Red-black graph

  @return bool
*/
bool exists(const RBVertex &v, const RBGraph &g);


/**
  @brief Return true if a vertex named \e name exists in \e g .

  @param[in] name    Vertex name
  @param[in] g    Red-black graph

  @return bool
*/
bool exists(const std::string &name, const RBGraph &g);


/**
  @brief Return true if \e g is empty .

  A red-black graph is empty if it has no vertices.

  @param[in] g Red-black graph

  @return bool
*/
inline bool is_empty(const RBGraph& g) { return (num_vertices(g) == 0); }


/**
  @brief Build the map in \e g .

  @param[in] g Red-black graph
*/
void build_vertex_map(RBGraph& g);


/**
  @brief Remove singleton vertices from \e g .

  A vertex is a singleton in a red-black graph if it has no incident edges.

  @param[in,out] g Red-black graph
*/
void remove_singletons(RBGraph& g);


/**
  @brief Return true if \e v is a species in \e g .

  @param[in] v Vertex
  @param[in] g Red-black graph

  @return bool
*/
inline bool is_species(const RBVertex& v, const RBGraph& g) {
  return (g[v].type == Type::species);
}


/**
  @brief Return true if \e v is a character in \e g .

  @param[in] v Vertex
  @param[in] g Red-black graph

  @return bool
*/
inline bool is_character(const RBVertex& v, const RBGraph& g) {
  return (g[v].type == Type::character);
}


/**
  @brief Return true if \e e is a black edge in \e g .

  @param[in] e Edge
  @param[in] g Red-black graph

  @return bool
*/
inline bool is_black(const RBEdge& e, const RBGraph& g) {
  return (g[e].color == Color::black);
}


/**
  @brief Return true if \e e is a red edge in \e g .

  @param[in] e Edge
  @param[in] g Red-black graph

  @return bool
*/
inline bool is_red(const RBEdge& e, const RBGraph& g) {
  return (g[e].color == Color::red);
}


/**
  @brief Copy graph \e g to graph \e g_copy .

  @param[in]     g      Red-black graph
  @param[in,out] g_copy Red-black graph
*/
void copy_graph(const RBGraph& g, RBGraph& g_copy);


/**
  @brief Copy graph \e g to graph \e g_copy and fill its vertex map.

  @param[in]     g      Red-black graph
  @param[in,out] g_copy Red-black graph
  @param[in,out] v_map  Vertex map, mapping vertices from g to g_copy
*/
void copy_graph(const RBGraph& g, RBGraph& g_copy, RBVertexMap& v_map);


/**
  @brief Overloading of operator<< for RBGraph.

  @param[in] os Output stream
  @param[in] g  Red-black graph

  @return Reference to the updated output stream
*/
std::ostream& operator<<(std::ostream& os, const RBGraph& g);


/**
  @brief Get the matrix representation \e m of \e g.

  @param[in] g  Red-black graph
  @param[in] m  Bool matrix containing the result
*/
void get_matrix_representation(const RBGraph& g, bool **m, size_t rows, size_t cols) ;


// File I/O

/**
  @brief Read from \e filename into \e g .

  @param[in]  filename Filename
  @param[out] g        Red-black graph
*/
void read_graph(const std::string& filename, RBGraph& g);


//=============================================================================
// Algorithm functions


/**
  @brief Return true if \e v is active in \e g .

  A vertex is active in a red-black graph if it's a character that is incident
  only on red edges or it's a species that is incident only on black edges.

  @param[in] v Vertex
  @param[in] g Red-black graph

  @return bool
*/
bool is_active(const RBVertex& v, const RBGraph& g);


/**
  @brief Return true if \e v is inactive in \e g .

  A vertex is inactive in a red-black graph if it's not active.

  @param[in] v Vertex
  @param[in] g Red-black graph

  @return bool
*/
inline bool is_inactive(const RBVertex& v, const RBGraph& g) {
  return !is_active(v, g);
}


/**
  @brief Return true if \e s is a pending species in \e g . 

  A species is pending in a red-black graph if it's a species that has just an incoming black edge and has a node degree equal to one.

  @param[in] s Species
  @param[in] g Red-black graph

  @return bool
*/
bool is_pending_species(const RBVertex& s, const RBGraph& g);


/**
  @brief Remove duplicate species from \e g . 

  Two species are duplicates if they are connected to the same species.

  @param[in] g Red-black graph
*/
void remove_duplicate_species(RBGraph& g);


/**
  @brief Return true if all the species in \e g have a red edge. 

  @param[in] g Red-black graph

  @return bool
*/
bool all_species_with_red_edges(const RBGraph& g);


/**
  @brief Return the list of pending species in \e g. 

  A species is pending in a red-black graph if it's a species that has just an incoming black edge and has a node degree equal to one.

  @param[in] g Red-black graph

  @return List
*/
std::list<RBVertex> get_pending_species(const RBGraph& g);


/**
  @brief Return true if \e v is red-universal in \e g .

  A vertex is red-universal in a red-black graph if it's an active character that is connected to all the species in \e g .

  @param[in] v Vertex
  @param[in] g Red-black graph

  @return bool
*/
bool is_red_universal(const RBVertex& v, const RBGraph& g);


/**
  @brief Return true if \e v is universal in \e g .

  A vertex is universal in a red-black graph if it's an inactive character that is connected to all the species in \e g .

  @param[in] v Vertex
  @param[in] g Red-black graph

  @return bool
*/
bool is_universal(const RBVertex v, const RBGraph& g);


/**
  @brief Build the red-black subgraphs of \e g .
         Each subgraph IS A COPY of the respective connected component and this means that the RBVertex objects of \e g will be different to those of the just-generated components. Thus, the mapping of the verteces between \e g and the components must be done by exploiting the vertex names.

  In addition, if the graph \e g is connected, RBGraphVector will be of size 1, but the unique_ptr will be empty. This is because the purpose of the functions is to build the subgraphs, not copy the whole graph when it isn't needed.

  @param[in] g Red-black graph

  @return Componentt vector of unique pointers to each subgraph
*/
RBGraphVector connected_components(const RBGraph& g);


/**
  @brief Build the red-black subgraphs of \e g.
         Each subgraph IS A COPY of the respective connected component and this means that the RBVertex objects of \e g will be different to those of the just-generated components. Thus, the mapping of the verteces between \e g and the components can be done by exploiting the vertex names.

  In addition, if the graph \e g is connected, RBGraphVector will be of size 1, but the unique_ptr will be empty. This is because the purpose of the functions is to build the subgraphs, not copy the whole graph when it isn't needed.

  @param[in] g       Red-black graph
  @param[in] c_map   Components map of \e g
  @param[in] c_count Number of connected components of \e g

  @return Component vector of unique pointers to each subgraph
*/
RBGraphVector connected_components(const RBGraph& g, const RBVertexIMap& c_map,
                                   const size_t c_count);


/**
  @brief Return the list of neighbors of species v.

  The neighbor of a species v in the graph is a species v0 that share
  some inactive character with v.
  
  @param[in] v            Species
  @param[in] g            Red-black graph

  @return  List
*/
std::list<RBVertex> get_neighbors(const RBVertex& v, const RBGraph& g);


/**
  @brief Return a map M where M[v] is the list of verteces adjacent to vertex \e v.
 
  @param[in] g            Red-black graph

  @return  Map
*/
std::map<RBVertex, std::list<RBVertex>> get_adj_map(const RBGraph& g);


/**
  @brief Return the list of vertices adjacent to character \e v.
 
  @param[in] v           RBVertex
  @param[in] g            Red-black graph

  @return  List
*/
std::list<RBVertex> get_adj_vertices(const RBVertex& v, const RBGraph& g);


/**
  @brief Given a species \e s, it returns the set of active characters adjacent to \e s

  @param[in] s Species in the graph
  @param[in] g Red-black graph

  @return Set of active characters
**/  
std::list<RBVertex> get_adj_active_characters(const RBVertex& s, const RBGraph& g);


/**
  @brief Given a species \e s, it returns the set of inactive characters adjacent to \e s .

  @param[in] s Species in the graph
  @param[in] g Red-black graph

  @return Set of inactive characters
**/  
std::list<RBVertex> get_adj_inactive_characters(const RBVertex& s, const RBGraph& g);


/**
  @brief Return true if \e v is in \e v_list .

  @param[in] v_list   a list of verteces
  @param[in] v        a vertex

  @return bool
*/
inline bool contains(const std::list<RBVertex>& v_list, const RBVertex& v) {
  return std::find(v_list.begin(), v_list.end(), v) != v_list.end();
}


/**
  @brief Return true if the set of inactive characters of \e s1 include the set of inactive characters of \e s2 . More formally, return true if IC(s1) ⊇ IC(s2).

  @param[in] s1         Vertex species
  @param[in] s2         Vertex species
  @param[in] g          Red black graph

  @return bool
*/
bool includes_species(const RBVertex& s1, const RBVertex& s2, const RBGraph& g);


/**
  @brief Return true if the set of species adjacent to \e c1 include the set of species adjacent to \e c2 . More formally, return true if S(c1) ⊇ S(c2).  

  @param[in] c1         Vertex character
  @param[in] c2         Vertex character
  @param[in] g          Red black graph

  @return bool
*/
bool includes_characters(const RBVertex& c1, const RBVertex& c2, const RBGraph& g);


/**
  @brief Return true if the characters of \e s1 overlap with the ones of \e s2, false otherwise.

  @param[in] s1         Vertex species
  @param[in] s2         Vertex species
  @param[in] g          Red black graph

  @return bool
*/
bool overlaps_species(const RBVertex& s1, const RBVertex& s2, const RBGraph& g);


/**
  @brief Return true if the species adjacent to \e c1 overlaps with the species adjacent to \e c2, false otherwise. More formally, return true if S(c1) ⊄ S(c2) and S(c2) ⊄ S(c1).

  @param[in] c1         Vertex character
  @param[in] c2         Vertex character
  @param[in] g          Red black graph

  @return bool
*/
bool overlaps_character(const RBVertex& c1, const RBVertex& c2, const RBGraph& g);


/**
  @brief Return the list of inactive characters in \e g .

  @param[in] g  A red-black graph

  @return List
*/
std::list<RBVertex> get_inactive_chars(const RBGraph& g);


/**
  @brief Build the list of maximal inactive characters of \e g .

  Let c be an unsigned character.
  Then S(c) is the set of species that have the character c.
  Then a character c is maximal in a red-black graph if S(c) ⊄ S(c') for any
  character c' of the graph.

  @param[in] g Red-black graph

  @return List
*/
const std::list<RBVertex> maximal_characters(const RBGraph& g);


/**
  @brief Build the maximal reducible red-black graph of \e g .

  Let GRB be a red-black graph and CM the set of maximal characters of GRB.
  A maximal reducible graph consists of a reducible red-black graph such that
  it consists only of maximal characters and has no active characters.
  The induced graph GRB|CM is a maximal reducible graph.

  @param[in] g      Red-black graph
  @param[in] active True: keep all active characters from \e g (GRB|CM∪A);
                    False: ignore all active characters from \e g (GRB|CM).

  @return Maximal reducible graph
*/
void maximal_reducible_graph(const RBGraph& g, RBGraph& gm, const bool active = false);


/**
  @brief Return true if \e g contains a red Σ-graph.

  A red-black graph containing a red Σ-graph cannot be reduced to an empty
  graph by a c-reduction. In other words, let c1 and c2 be two characters and s1, s2 and s3 be three species, then a red Σ-graph is a path of length 4 through c1, c2, s1, s2 and s3 consisting of only red edges. The red Σ-graph corresponds to the forbidden configuration.

  @param[in] g Red-black graph

  @return bool
*/
bool has_red_sigmagraph(const RBGraph& g);


/**
  @brief Check if \e g contains a red Σ-graph with characters \e c0 and \e c1 .

  This function should only be called by \e has_redsigma.

  @param[in] c0 Vertex
  @param[in] c1 Vertex
  @param[in] g  Red-black graph

  @return bool
*/
bool has_red_sigmapath(const RBVertex c0, const RBVertex c1, const RBGraph& g);


/**
  @brief Change the type of vertex \e v from \e active to \e inactive or viceversa.

  A vertex is active in a red-black graph if it's a character that is incident
  only on red edges.

  @param[in] v Vertex
  @param[in] g Red-black graph
*/
void change_char_type(const RBVertex& v, RBGraph& g);


/**
  @brief Return the list of species in the connected component of \e g to which \e v belongs if \e v is a character, otherwise, if \e v is a species, return the list of characters in the connected component of \e g to which \e v belongs.

  @param[in] v Vertex in the red-black graph
  @param[in] g Red-black graph

  @return List
**/  
std::list<RBVertex> get_comp_vertex(const RBVertex& v, const RBGraph& g);


/**
  @brief Return the list of active characters of a red-black graph.

  @param[in] g Red-black graph
 
  @return List
**/
std::list<RBVertex> get_active_chars(const RBGraph& g);


/**
  @brief Return the list of active species of a red-black graph.

  @param[in] g Red-black graph
 
  @return List
**/
std::list<RBVertex> get_active_species(const RBGraph& g);


/**
  @brief Return the active characters included in the same component of \e s .

  @param[in] s Species
  @param[in] g Red-black graph

  @return List
**/
std::list<RBVertex> get_comp_active_characters(const RBVertex s, const RBGraph& g);


/**
  @brief Return true if \e g is degenerate.

  A graph \e g is degenerate if it consists of species having all inactive
  characters excepts for one and has at least an active character.

  @param[in] g Red-black graph

  @return bool
**/
bool is_degenerate(const RBGraph& g);


/**
  @brief It builds the minimal form graph \e gmf of \e g

  @param[in] g Red-black graph
  @param[in] gmf Red-black graph

  @return void
**/
void minimal_form_graph(const RBGraph& g, RBGraph& gmf);


/**
  @brief It returns true if the matrix associated with \e g has the 01 property

  @param[in] g Red-black graph

  @return bool
**/
bool has_consecutive_ones_property(RBGraph& g);

#endif  // RBGRAPH_HPP

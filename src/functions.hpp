/** 
 * 
 * @file functions.hpp
 * 
 * @author Simone Paolo Mottadelli
 * 
 */

#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include "rbgraph.hpp"

// ************************************************
// ************************************************
// ********** Auxiliary data structures ***********

/**
  Scoped enumeration type whose underlying size is 1 byte, used for character
  state.

  State is paired with a character in the struct SignedCharacter.
*/
enum class State : bool {
  lose,  ///< The paired character is lost
  gain   ///< The paired character is gained
};

/**
  @brief Struct used to represent a signed character

  Each character c+ and c− is called a signed character.
*/
struct SignedCharacter {
  std::string character{};    ///< Character name
  State state = State::gain;  ///< Character state
};

/**
  @brief Overloading of operator<< for State

  @param[in] os Output stream
  @param[in] s  State

  @return Updated output stream
*/
inline std::ostream& operator<<(std::ostream& os, const State s) {
  const bool sign = (s == State::lose);

  return os << (sign ? "-" : "+");
}


/**
  @brief Overloading of operator<< for SignedCharacter

  @param[in] os Output stream
  @param[in] sc SignedCharacter

  @return Updated output stream
*/
inline std::ostream& operator<<(std::ostream& os, const SignedCharacter sc) {
  return os << sc.character << sc.state;
}

/**
  @brief Overloading of operator== for a pair of signed characters

  @param[in] a SignedCharacter
  @param[in] b SignedCharacter

  @return True if a is equal to b
*/
inline bool operator==(const SignedCharacter& a, const SignedCharacter b) {
  return (a.character == b.character && a.state == b.state);
}

// ************************************************
// ************************************************

/**
  @brief Realize the character \e c (+ or -) in \e g

  Returns the list of realized characters and bool = True if the realization of
  \e sc is feasible for \e g.
  Returns an empty list and bool = False otherwise.

  Let GRB be a red-black graph, and let c be a character of GRB.
  Let D(c) be the set of species in the connected component of GRB that
  contains c.
  The result of the realization of c+ on GRB, which is defined only if c is
  inactive, is a red-black graph obtained from GRB by adding a red edge between
  c and each species in D(c) \ N(c), deleting all black edges incident on c,
  and finally deleting all isolated vertices.
  The realization of c− on GRB is defined only if c is active and there is no
  species in D(c) \ N(c): in this case the results of the realization is
  obtained from GRB by deleting all edges incident on c, and then deleting all
  isolated vertices.

  @param[in]     sc SignedCharacter of \e g
  @param[in,out] g  Red-black graph

  @return Realized characters (list of signed characters).
          If the realization was successful then the bool flag will be true.
          When the flag is false, the returned list is empty
*/
std::pair<std::list<SignedCharacter>, bool> realize_character(const SignedCharacter& sc, RBGraph& g);


/**
  @brief Realize the inactive characters of the species \e v in \e g

  Returns the list of realized characters and bool = True if the realization of
  \e v is feasible for \e g.
  Returns an empty list and bool = False otherwise.

  The realization of a species s is the realization of its set C(s) of
  characters in any order. An active character c that is connected to all
  species of a graph GRB by red edges is called free in GRB and it is then
  deleted from GRB.

  @param[in]     v Vertex (a species)
  @param[in,out] g Red-black graph

  @return Realized characters (list of signed characters).
          If the realization was successful then the bool flag will be true.
          When the flag is false, the returned list is empty
*/
std::pair<std::list<SignedCharacter>, bool> realize_species(const RBVertex v,
                                                    RBGraph& g);


/**
  @brief Realize the list of characters \e lsc (+ or - each) in \e g

  Returns the list of realized characters and bool = True if the realizations
  of \e lsc are feasible for \e g.
  Returns an empty list and bool = False otherwise.

  @param[in]     lsc List of signed characters of \e g
  @param[in,out] g   Red-black graph

  @return Realized characters (list of signed characters).
          If the realizations were successful then the bool flag will be true.
          When the flag is false, the returned list is empty
*/
std::pair<std::list<SignedCharacter>, bool> realize(
    const std::list<SignedCharacter>& lsc, RBGraph& g);

bool is_complete(std::list<SignedCharacter> sc, const RBGraph& gm);


/**
  @brief Given a list of vertices, it sorts the list by using the concept of vertex degree  

  @param[in] list_to_order  List of vertices
  @param[in] g              Red-black graph
*/
void sort_by_degree_desc(std::list<RBVertex>& list_to_sort, const RBGraph& g);

/**
  @brief Given a list of vertices, it sorts the list by using the concept of vertex degree  

  @param[in] list_to_order  List of vertices
  @param[in] g              Red-black graph
*/
void sort_by_degree_asc(std::list<RBVertex>& list_to_sort, const RBGraph& g);

/**
  @brief Return the minimal p-active species in \e g.
  
  If \e all is set to false, then the output list will contain only the first minimal p-active species encountered.

  @param[in] g Red-black graph
  @param[in] all bool

  @return The list of minimal p-active species
*/
std::list<RBVertex> get_all_minimal_p_active_species(const RBGraph& g, bool all=true);


/**
  @brief Return the minimal p-active species in \e g. 

  @param[in] g Red-black graph

  @return The minimal p-active species
*/
RBVertex get_minimal_p_active_species(const RBGraph& g);


/**
  @brief Return a quasi-active species in \e g if it exists.

  A vertex \e s is quasi-active in a red-black graph if it has red-incoming edges, but it can be removed from \e g without producing a red-Σ-graph.

  @param[in] g Red-black graph

  @return Vertex
*/
RBVertex get_quasi_active_species(const RBGraph& g);

/**
  @brief Return a pending species that has a unique minimal neighbor. This function is called in the algorithm when there are exactly two pending species.

  @param[in] s a RBVertex as species

  @return A species
*/
RBVertex get_pending_and_minimal_p_active_species_with_unique_closest_neighbor(const RBGraph& g);

/**
  @brief Execute the algorithm on the maximal reducible graph \e g .
  It returns the sequence of the realized characters.

  @param[in] g Red-black graph

  @return List
*/
std::list<SignedCharacter> ppp_maximal_reducible_graphs(RBGraph& g);


/**
  @brief Realize the characters in \e g that are red-universal or universal.

  @param[in,out] g Red-black graph

  @return Realized characters (list of signed characters)
*/
std::pair<std::list<SignedCharacter>, bool> realize_red_univ_and_univ_chars(RBGraph& g);


/**
  @brief Return the extension of a species \e s.

  @param[in] s A RBVertex as species
  @param[in] gmax Red-black graph (maximal reducible graph)
  @param[in] gmin Red-black graph (minimal form graph)

  @return The extension of \e s.
*/
RBVertex get_extension(const RBVertex& s, const RBGraph& gmax, const RBGraph& gmin);


/**
  @brief Returns the sources in the maximal reducible graph \e gm.

  @param[in] gm Red-black graph (maximal reducible graph)

  @return The sources of \e gm.
*/
std::list<RBVertex> get_sources(const RBGraph& gm);


/**
  @brief Returns true if gm is 2-solvable

  @param[in] sources List of sources
  @param[in] gm Red-black graph (maximal reducible graph))

  @return bool
*/
bool is_2_solvable(std::list<RBVertex>& sources, const RBGraph& gm);


/**
  @brief It returns the closure CL of \e v. In particular, a ∈ CL(v) if and only if a is minimal in G and moreover it is included in all maximal characters of \e v.

  @param[in] v Species
  @param[in] g Red-black graph

  @return List
**/
std::list<RBVertex> closure(const RBVertex& v, const RBGraph& g);

#endif

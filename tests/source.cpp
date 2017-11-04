#include "functions.hpp"


int main(int argc, const char* argv[]) {
  HDGraph hasse;
  RBGraph g;
  
  read_graph("tests/no_5x2.txt", g);
  hasse_diagram(hasse, g);
  
  HDVertexIter v, v_end, a, b, c;
  std::tie(v, v_end) = vertices(hasse);
  a = find_source(v, v_end, hasse);
  v = a; ++v;
  b = find_source(v, v_end, hasse);
  v = b; ++v;
  c = find_source(v, v_end, hasse);
  std::tie(v, v_end) = vertices(hasse);
  
  assert(hasse[*a].species == std::list<std::string>({ "s1" }));
  assert(hasse[*b].species == std::list<std::string>({ "s3" }));
  assert(c == v_end);
  assert(safe_source(*a, g, hasse) && safe_source(*b, g, hasse));
  
  std::cout << "source: tests passed" << std::endl;
  
  return 0;
}

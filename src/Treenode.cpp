#include "Treenode.hpp"

Treenode::Treenode(std::string directive, std::vector<std::string> value)
{
  this->directive = directive;
  this->value = value;
}
void Treenode::add(Treenode *node) { children.push_back(node); }
// void Treenode::print(int level = 0)
// {
//   for (int i = 0; i < level; ++i)
//     std::cout << "  ";
//   std::cout << directive;
//   if (!value.empty())
//   {
//     size_t i = 0;
//     while (i < value.size())
//     {
//       std::cout << " " << value[i];
//       i++;
//     }
//   }
//   std::cout << std::endl;
//   for (size_t i = 0; i < children.size(); ++i)
//     children[i]->print(level + 1);
// }
std::string &Treenode::getDirective() { return directive; }
std::vector<std::string> &Treenode::getValue() { return value; }
std::vector<Treenode *> &Treenode::getChildren() { return children; }

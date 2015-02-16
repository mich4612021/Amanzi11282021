#ifndef XMLPARAMETERLISTWRITER_H
#define XMLPARAMETERLISTWRITER_H

#include "Teuchos_ParameterList.hpp"
#include "Teuchos_XMLObject.hpp"
#include "Teuchos_Utils.hpp"
#include "Teuchos_ArrayView.hpp"

namespace Teuchos {

#if 0
template<>
class ToStringTraits<double> {
 public:
  static std::string toString (const double& t) {
    std::ostringstream os;
    os.setf (std::ios::scientific);
    os.precision (17);
    os << t;
    return os.str();
  }
};
#endif

class Amanzi_XMLParameterListWriter {
 public:
  Amanzi_XMLParameterListWriter();
  XMLObject toXML(const ParameterList& p) const;
    
  inline std::string Amanzi_toString(const Teuchos::Array<double>& a) const {
    std::ostringstream ss;
    ss.setf(std::ios::scientific);
    ss.precision(precision_);

    ss << "{";
    for (int i=0; i < a.size(); ++i) {
      ss << a[i];
      if (i < a.size()-1) ss << ", ";
    }
    ss << "}";
    
    return ss.str();
  }

  void set_precision(int i) { precision_ = i; }

  void unused(const ParameterList& p, std::ostream& os) const;
  
 private:
  XMLObject toXML(const ParameterEntry& p) const;

 private:
  int precision_;
};

}  // namespace Teuchos
 
#endif


#include "miniScene/Scene.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <fstream>

using namespace mini;

#define BAKE_TRANSFORMS 1

Scene::SP g_scene;
affine3f g_xfm;

void parse_Other(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            printf("node type: Element, name: %s\n", cur_node->name);
        }

        //        print_element_names(cur_node->children);
    }
}

void parse_AffineSpace(xmlNode *root, const std::vector<uint8_t> &binData)
{
  const std::string value = (const char *)xmlNodeListGetString(root->doc, root, 1);
  affine3f xfm;
  sscanf(value.c_str(),
         "%f %f %f %f %f %f %f %f %f %f %f %f",
         &xfm.l.vx.x,
         &xfm.l.vy.x,
         &xfm.l.vz.x,
         &xfm.p.x,
         &xfm.l.vx.y,
         &xfm.l.vy.y,
         &xfm.l.vz.y,
         &xfm.p.y,
         &xfm.l.vx.z,
         &xfm.l.vy.z,
         &xfm.l.vz.z,
         &xfm.p.z);
  g_xfm = g_xfm * xfm;
//   PRINT(value);
}

// TriangleMesh/positions
std::vector<vec3f> parse_positions(xmlNode *root, const std::vector<uint8_t> &binData)
{
  size_t ofs=0, size=0;
  for (xmlAttr *prop = root->properties; prop; prop = prop->next) {
    const std::string key = (const char *)prop->name;
    const std::string value = (const char *)xmlNodeListGetString(root->doc, prop->children, 1);
    if (key == "ofs")
      ofs = std::stol(value);
    else if (key == "size")
      size = std::stol(value);
    else
      throw std::runtime_error("un-recognized attributed "+key+" = "+value+" in TriangleMesh/triangles");
  }
  std::vector<vec3f> data(size);
  memcpy(data.data(),binData.data()+ofs,size*sizeof(data[0]));
  return data;
}

// TriangleMesh/normals
std::vector<vec3f> parse_normals(xmlNode *root, const std::vector<uint8_t> &binData)
{
  size_t ofs=0, size=0;
  for (xmlAttr *prop = root->properties; prop; prop = prop->next) {
    const std::string key = (const char *)prop->name;
    const std::string value = (const char *)xmlNodeListGetString(root->doc, prop->children, 1);
    if (key == "ofs")
      ofs = std::stol(value);
    else if (key == "size")
      size = std::stol(value);
    else
      throw std::runtime_error("un-recognized attributed "+key+" = "+value+" in TriangleMesh/triangles");
  }
  std::vector<vec3f> data(size);
  memcpy(data.data(),binData.data()+ofs,size*sizeof(data[0]));
  return data;
}

// TriangleMesh/texoords
std::vector<vec2f> parse_texcoords(xmlNode *root, const std::vector<uint8_t> &binData)
{
  size_t ofs=0, size=0;
  for (xmlAttr *prop = root->properties; prop; prop = prop->next) {
    const std::string key = (const char *)prop->name;
    const std::string value = (const char *)xmlNodeListGetString(root->doc, prop->children, 1);
    if (key == "ofs")
      ofs = std::stol(value);
    else if (key == "size")
      size = std::stol(value);
    else
      throw std::runtime_error("un-recognized attributed "+key+" = "+value+" in TriangleMesh/triangles");
  }
  std::vector<vec2f> data(size);
  memcpy(data.data(),binData.data()+ofs,size*sizeof(data[0]));
  return data;
}
// TriangleMesh/triangles
std::vector<vec3i> parse_triangles(xmlNode *root, const std::vector<uint8_t> &binData)
{
  size_t ofs=0, size=0;
  for (xmlAttr *prop = root->properties; prop; prop = prop->next) {
    const std::string key = (const char *)prop->name;
    const std::string value = (const char *)xmlNodeListGetString(root->doc, prop->children, 1);
    if (key == "ofs")
      ofs = std::stol(value);
    else if (key == "size")
      size = std::stol(value);
    else
      throw std::runtime_error("un-recognized attributed "+key+" = "+value+" in TriangleMesh/triangles");
  }
  std::vector<vec3i> data(size);
  memcpy(data.data(),binData.data()+ofs,size*sizeof(data[0]));
  return data;
}

vec3f get3f(const std::string &s)
{
  vec3f v;
  sscanf(s.c_str(),"%f %f %f",&v.x,&v.y,&v.z);
  return v;
}
float get1f(const std::string &s)
{
  float v;
  sscanf(s.c_str(),"%f",&v);
  return v;
}


xmlNode *findParams(xmlNode *root)
{
  for (xmlNode *node = root; node; node = node->next) {
    if (node->type != XML_ELEMENT_NODE)
      continue;
    const std::string type = (const char *)node->name;
    if (type == "code")
      continue;
    if (type == "parameters")
      return node->children;
    throw std::runtime_error("unexpected tag '"+type+"' in material...");
  }
  throw std::runtime_error("could not find <parameters> in material...");
}

std::string getName(xmlNode *param)
{
  for (xmlAttr *prop = param->properties; prop; prop = prop->next) {
    const std::string key = (const char *)prop->name;
    const std::string value = (const char *)xmlNodeListGetString(param->doc, prop->children, 1);
    if (key == "name")
      return value;
  }
  throw std::runtime_error("could not find 'name' of material parameter");
}

Material::SP parse_Metal(xmlNode *root)
{
  Metal::SP mat = Metal::create();
  for (xmlNode *param = findParams(root); param; param = param->next) {
    if (param->type != XML_ELEMENT_NODE)
      continue;
    const std::string name  = getName(param);
    const std::string type  = (const char *)param->name;
    const std::string value = (const char *)xmlNodeListGetString(root->doc, param->children, 1);
    if (name == "eta") {
      mat->eta = get3f(value);
      continue;
    }
    if (name == "k") {
      mat->k = get3f(value);
      continue;
    }
    if (name == "roughness") {
      mat->roughness = get1f(value);
      continue;
    }
      
    throw std::runtime_error("un-handled param "+type+" "+name+" = "+value);
  }
  return mat;
}

Material::SP parse_Plastic(xmlNode *root)
{
  Plastic::SP mat = Plastic::create();
  for (xmlNode *param = findParams(root); param; param = param->next) {
    if (param->type != XML_ELEMENT_NODE)
      continue;
    const std::string name  = getName(param);
    const std::string type  = (const char *)param->name;
    const std::string value = (const char *)xmlNodeListGetString(root->doc, param->children, 1);
    if (name == "Ks") {
      mat->Ks = get3f(value);
      continue;
    }
    if (name == "pigmentColor") {
      mat->pigmentColor = get3f(value);
      continue;
    }
    if (name == "eta") {
      mat->eta = get1f(value);
      continue;
    }
    if (name == "roughness") {
      mat->roughness = get1f(value);
      continue;
    }
      
    throw std::runtime_error("un-handled param "+type+" "+name+" = "+value);
  }
  return mat;
}

Material::SP parse_MetallicPaint(xmlNode *root)
{
  MetallicPaint::SP mat = MetallicPaint::create();
  for (xmlNode *param = findParams(root); param; param = param->next) {
    if (param->type != XML_ELEMENT_NODE)
      continue;
    const std::string name  = getName(param);
    const std::string type  = (const char *)param->name;
    const std::string value = (const char *)xmlNodeListGetString(root->doc, param->children, 1);
    if (name == "eta") {
      mat->eta = get1f(value);
      continue;
    }
    if (name == "glitterColor") {
      mat->glitterColor = get3f(value);
      continue;
    }
    if (name == "shadeColor") {
      mat->shadeColor = get3f(value);
      continue;
    }
    if (name == "glitterSpread") {
      mat->glitterSpread = get1f(value);
      continue;
    }
      
    throw std::runtime_error("un-handled param "+type+" "+name+" = "+value);
  }
  return mat;
}

Material::SP parse_Matte(xmlNode *root)
{
  Matte::SP mat = Matte::create();
  for (xmlNode *param = findParams(root); param; param = param->next) {
    if (param->type != XML_ELEMENT_NODE)
      continue;
    const std::string name  = getName(param);
    const std::string type  = (const char *)param->name;
    const std::string value = (const char *)xmlNodeListGetString(root->doc, param->children, 1);
    if (name == "reflectance") {
      mat->reflectance = get3f(value);
      continue;
    }
      
    throw std::runtime_error("un-handled param "+type+" "+name+" = "+value);
  }
  return mat;
}

Material::SP parse_Dielectric(xmlNode *root)
{
  Dielectric::SP mat = Dielectric::create();
  for (xmlNode *param = findParams(root); param; param = param->next) {
    if (param->type != XML_ELEMENT_NODE)
      continue;
    const std::string name  = getName(param);
    const std::string type  = (const char *)param->name;
    const std::string value = (const char *)xmlNodeListGetString(root->doc, param->children, 1);
    if (name == "transmission") {
      mat->transmission = get3f(value);
      continue;
    }
    if (name == "etaInside") {
      mat->etaInside = get1f(value);
      continue;
    }
    if (name == "etaOutside") {
      mat->etaOutside = get1f(value);
      continue;
    }
      
    throw std::runtime_error("un-handled param "+type+" "+name+" = "+value);
  }
  return mat;
}

Material::SP parse_ThinGlass(xmlNode *root)
{
  ThinGlass::SP mat = ThinGlass::create();
  for (xmlNode *param = findParams(root); param; param = param->next) {
    if (param->type != XML_ELEMENT_NODE)
      continue;
    const std::string name  = getName(param);
    const std::string type  = (const char *)param->name;
    const std::string value = (const char *)xmlNodeListGetString(root->doc, param->children, 1);
    if (name == "transmission") {
      mat->transmission = get3f(value);
      continue;
    }
    if (name == "eta") {
      mat->eta = get1f(value);
      continue;
    }
    if (name == "thickness") {
      mat->thickness = get1f(value);
      continue;
    }
      
    throw std::runtime_error("un-handled param "+type+" "+name+" = "+value);
  }
  return mat;
}

// TriangleMesh/material
Material::SP parse_material(xmlNode *root, const std::vector<uint8_t> &binData)
{
  // <material>
  //   <code>"Metal"</code>
  //   <parameters>
  //     <float3 name="eta">0.62 0.62 0.62</float3>
  //     <float3 name="k">4.8 4.8 4.8</float3>
  //     <float name="roughness">0</float>
  //   </parameters>
  // </material>
  
  // std::string code = "";
  for (xmlNode *node = root; node; node = node->next) {
    if (node->type != XML_ELEMENT_NODE)
      continue;
    const std::string type = (const char *)node->name;
    if (type == "code") {
      const std::string code
        = (const char *)xmlNodeListGetString(root->doc, node->children, 1);
      try {
        if (code == "\"Metal\"") {
          return parse_Metal(root);
        } else if (code == "\"MetallicPaint\"") {
          return parse_MetallicPaint(root);
        } else if (code == "\"Plastic\"") {
          return parse_Plastic(root);
        } else if (code == "\"Matte\"") {
          return parse_Matte(root);
        } else if (code == "\"Dielectric\"") {
          return parse_Dielectric(root);
        } else if (code == "\"ThinGlass\"") {
          return parse_ThinGlass(root);
        } else
          throw std::runtime_error("unknown material code '"+code+"'");
      } catch (std::exception &e) {
        throw std::runtime_error("error parsing material code '"+code+"' : "+e.what());
        
      }
    }
  }
  throw std::runtime_error("could not find material '<code>' tag");
    // } else if (type == "parameters") {
    //   for (xmlNode *param = node->children; param; param = param->next) {
    //     std::string name;
    //     for (xmlAttr *prop = param->properties; prop; prop = prop->next) {
    //       const std::string key = (const char *)prop->name;
    //       const std::string value = (const char *)xmlNodeListGetString(root->doc, prop->children, 1);
    //       if (key == "name")
    //         name = value;
    //     }
    //     if (param->type != XML_ELEMENT_NODE)
    //       continue;
    //     const std::string type = (const char *)param->name;
    //     const std::string value = (const char *)xmlNodeListGetString(root->doc, param->children, 1);

    //     if (code == "Metal") {
    //       if (name == "k") {
    //         // ????
    //         mat->baseColor = get3f(value);
    //         continue;
    //       }
    //       if (name == "eta") {
    //         // ????
    //         mat->metallic = get3f(value).x;
    //         continue;
    //       }
    //       if (name == "roughness") {
    //         // ????
    //         mat->metallic = get1f(value);
    //         continue;
    //       }
    //     }
    //     if (code == "MetallicPaint") {
    //       if (name == "eta") {
    //         // ????
    //         mat->metallic = get1f(value);
    //         continue;
    //       }
    //       if (name == "glitterColor") {
    //         // ????
    //         // ignoring!?
    //         continue;
    //       }
    //       if (name == "glitterSpread") {
    //         // ????
    //         // ignoring!?
    //         continue;
    //       }
    //       if (name == "shadeColor") {
    //         // ????
    //         mat->baseColor = get3f(value);
    //         continue;
    //       }
    //     }
    //     if (code == "Plastic") {
    //       if (name == "Ks") { // 3f, apparently 1,1,1
    //         // ????
    //         continue;
    //       }
    //       if (name == "pigmentColor") { // 3f, apparently 0.05
    //         // ????
    //         mat->baseColor = get3f(value);
    //         continue;
    //       }
    //       if (name == "eta") { // 1f, apparently 1.5
    //         // ????
    //         continue;
    //       }
    //       if (name == "roughness") { // 1f, apparently 0.1
    //         // ????
    //         mat->roughness = get1f(value);
    //         continue;
    //       }
    //     }
    //     if (code == "Matte") {
    //       if (name == "reflectance") { // 3f, apparently 1,1,1
    //         // ????
    //         mat->baseColor = get3f(value);
    //         continue;
    //       }
    //     }
    //     if (code == "Dielectric") {
    //       if (name == "etaInside") { // 1f, apparently 1.45
    //         mat->ior = get1f(value);
    //         continue;
    //       }
    //       if (name == "etaOutside") { // 1f, apparently 1
    //         // mat->ior = get1f(value);
    //         //ignore
    //         continue;
    //       }
    //       if (name == "transmission") { // 3f, apparently .95
    //         mat->baseColor = get3f(value);
    //         continue;
    //       }
    //     }
    //     if (code == "ThinGlass") {
    //       if (name == "eta") { // 1f, apparently 1.5
    //         mat->ior = get1f(value);
    //         continue;
    //       }
    //       if (name == "transmission") { // 3f, apparently .95
    //         mat->baseColor = get3f(value);
    //         continue;
    //       }
    //       if (name == "thickness") { // 1f, apparently 1
    //         // ignore
    //         continue;
    //       }
    //     }
        
    //     throw std::runtime_error("un-handled material "+code+" param "+type+" "+name+" = "+value);
    //     // PRINT(type);
    //     // PRINT(name);
    //     // PRINT(value);
  //     }
  //   } else 
  //     throw std::runtime_error("unknown material node type '"+type+"'");
  // }
  // return mat;
}

void parse_TriangleMesh(xmlNode *root, const std::vector<uint8_t> &binData)
{
  Mesh::SP mesh = Mesh::create();
  
  for (xmlNode *node = root; node; node = node->next) {
    if (node->type != XML_ELEMENT_NODE)
      continue;
    const std::string type = (const char *)node->name;
    if (type == "environment") {
      // IGNORE
    } else if (type == "positions") {
      mesh->vertices = parse_positions(node,binData);
    } else if (type == "normals") {
      mesh->normals = parse_normals(node,binData);
    } else if (type == "texcoords") {
      mesh->texcoords = parse_texcoords(node,binData);
    } else if (type == "triangles") {
      mesh->indices = parse_triangles(node,binData);
    } else if (type == "material") {
      parse_material(node->children,binData);
    } else
      throw std::runtime_error("unknown Mesh node type '"+type+"'");
  }
  if (mesh->indices.empty()) return;

#if BAKE_TRANSFORMS
  for (auto &v : mesh->vertices)
    v = xfmPoint(g_xfm,v);
  for (auto &n : mesh->normals)
    n = xfmNormal(g_xfm,n);
  g_scene->instances.push_back(Instance::create(Object::create({mesh})));
#else
  g_scene->instances.push_back(Instance::create(Object::create({mesh}),g_xfm));
#endif
}

void parse_Group(xmlNode *root, const std::vector<uint8_t> &binData);
void parse_Transform(xmlNode *root, const std::vector<uint8_t> &binData)
{
  affine3f saved_xfm = g_xfm;

  for (xmlNode *node = root; node; node = node->next) {
    if (node->type != XML_ELEMENT_NODE)
      continue;
    const std::string type = (const char *)node->name;
    if (type == "Transform") {
      parse_Transform(node->children, binData);
    } else if (type == "AffineSpace") {
      parse_AffineSpace(node->children, binData);
    } else if (type == "TriangleMesh") {
      parse_TriangleMesh(node->children, binData);
    } else if (type == "Group") {
      parse_Group(node->children, binData);
    } else
      throw std::runtime_error("unknown Transform node type '"+type+"'");
  }
  
  g_xfm = saved_xfm;
}


void parse_Group(xmlNode *root, const std::vector<uint8_t> &binData)
{
  for (xmlNode *node = root; node; node = node->next) {
    if (node->type != XML_ELEMENT_NODE)
      continue;
    const std::string type = (const char *)node->name;
    if (type == "Group") {
      parse_Group(node->children, binData);
    } else if (type == "Transform") {
      parse_Transform(node->children, binData);
    } else if (type == "TriangleMesh") {
      parse_TriangleMesh(node->children, binData);
    } else
      throw std::runtime_error("unknown scene node type '"+type+"'");
  }
}

void parse_scene(xmlNode *root, const std::vector<uint8_t> &binData)
{
  for (xmlNode *node = root; node; node = node->next) {
    if (node->type != XML_ELEMENT_NODE)
      continue;
    const std::string type = (const char *)node->name;
    if (type == "Group") {
      parse_Group(node->children,binData);
    } else
      throw std::runtime_error("unknown scene node type '"+type+"'");
  }
}

void parse_root(xmlNode *root, const std::vector<uint8_t> &binData)
{
  if (std::string((const char *)root->name) == "scene")
    parse_scene(root->children,binData);
  else
    throw std::runtime_error("not a BGFScene!?");
}

int main(int ac, char **av)
{
  
  std::string inFileName;
  std::string outFileName = "xmlbin2mini.mini";
  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg[0] != '-')
      inFileName = arg;
    else if (arg == "-o")
      outFileName = av[++i];
    else throw std::runtime_error("unknown cmdline arg "+arg);
  }

  g_scene = Scene::create();
  
  const std::string binFileName = inFileName.substr(0,inFileName.size()-3)+"bin";
  std::ifstream in(binFileName.c_str(),std::ios::binary);
  in.seekg(0, in.end);
  size_t fileSize = in.tellg();
  std::vector<uint8_t> binData(fileSize);
  in.seekg(0, in.beg);
  in.read((char *)binData.data(),fileSize);
  
  LIBXML_TEST_VERSION;
  xmlDoc *doc = xmlReadFile(inFileName.c_str(), NULL, 0);
  xmlNode *root = xmlDocGetRootElement(doc);
  parse_root(root,binData);
  xmlFreeDoc(doc);
  xmlCleanupParser();

  g_scene->save(outFileName.c_str());
  return 0;
}

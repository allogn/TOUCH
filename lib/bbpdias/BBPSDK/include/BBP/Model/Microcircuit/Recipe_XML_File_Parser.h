#ifndef BBP_RECIPE_XML_FILE_PARSER_H
#define BBP_RECIPE_XML_FILE_PARSER_H

#ifdef BBP_XML_SUPPORTED

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string>
#include <set>
#include <map>

#include "Types.h"
namespace bbp{
//! Class to read the recipe xml file and determine circuit parameters for use by other applications and the SDK
/*!
	This class reads the new recipe.xml file. The following parameters are determined currently:
	Number of Neurons, Number of Minicolumns, Number of Morphologies, A set of Morphology Types 

*/

class Recipe_XML_File_Parser{

	public:
		//! Constructor. 
		/*!
		 * Needs the xml files as inputs.Defaults to recipe.xml and connection_rules.xml as input
		 * The input is a std::string
		 */
		inline Recipe_XML_File_Parser(std::string recipe_path="./recipe.xml",std::string conn_path="./connection_rules.xml");

		//! Destructor
		/*!
		 * closes the xml parser and performs cleanup
		 */
		inline ~Recipe_XML_File_Parser();

		//! Returns total number of neurons 
		inline Count number_of_neurons() const;

		//! Returns number of minicolumns
		inline Count number_of_minicolumns() const;

		//! Returns number of Morphologies
		inline Count number_of_morphologies() const;

		//! Returns number of Layers
		inline Cortical_Layer_Number number_of_layers() const;


		//! Returns a std set<string> containg all the morphologies as specified in the recipe
		/*!
		 * Note that there is no ordering in this set(as in implicit in the definition of a set itself)
		 * Ordering information. when needed (for eg to get id of morphology type) must be read from the mvd file
		 */
		inline std::set<std::string> all_morphologies() const;

		//! Returns a std map containg probabilities of connection between layers in the recipe
		/*!
		 * The map is a std::map<Cortical_Cortical_Layer_Number , std::map<Cortical_Cortical_Layer_Number,Probability> >.
		 */
		inline std::map<Cortical_Layer_Number, std::map<Cortical_Layer_Number,Probability> > layer_connection_probabilities() const;

		//! Returns a std map containg probabilities of connection between morphologies in the recipe
		/*!
		 * The map is a std::map<Label, std::map<Label,Probability> >.
		 */
		inline std::map<Label, std::map<Label,Probability> > morphology_connection_probabilities() const;

		//! Returns a std map containg probabilities of connection between neuron types(exc-inh) in the recipe
		/*!
		 * The map is a std::map<Label, std::map<Label,Probability> >.
		 */
		inline std::map<Label, std::map<Label,Probability> > neurontype_connection_probabilities() const;


	private:
		//! Data members that holds pointers to the xml dom trees.These are provided by libxml.
		xmlDocPtr _doc,_doc2;
		//! Data members used internally for xml parsing
		xmlNode *_cur_node, *_child_node, *_child_node2;
		//! Data member that stores the morphologies in a set<string>
		std::set<std::string> _morph_set;
		//! Data member that stores number of neurons
		int _num_neurons;
		//! Data member that stores number of minicolumns
		int _num_mini;
		//! Data member that stores number of layers
		int _num_layers;
	
		//! Data member that stores the layer-layer connectivity
		std::map <Cortical_Layer_Number,std::map<Cortical_Layer_Number, Probability> > _layer_connectivity;
		//! Data member that stores the mtype-mtype connectivity
		std::map <Label,std::map<Label, Probability> > _mtype_connectivity;
		//! Data member that stores the neuron type(exc-inh) connectivity
		std::map <Label,std::map<Label, Probability> > _sclass_connectivity;

};

// ----------------------------------------------------------------------------------------



//IMPLEMENTATION
inline
Recipe_XML_File_Parser::Recipe_XML_File_Parser(std::string recipe_path,std::string conn_path)
:_num_layers(0)
/*! 
  \todo
	Replace error handling related printf's by exceptions
 */ 
	
{
	//
	//FIRST PARSE MAIN RECIPE FILE
	//
	_doc = xmlParseFile(recipe_path.c_str());

	if (_doc == NULL) 
		printf("error: could not parse file \n");

	/*Get the root element node */
	xmlNode *root = NULL;
	root = xmlDocGetRootElement(_doc);

	// Must have root element, a name and the name must be "blueColumn"

	if( !root || 
			!root->name ||
			xmlStrcmp(root->name,(const xmlChar*)"blueColumn") ) 
	{
		xmlFreeDoc(_doc);
	}
	// This for loop parses the xml doc and stores the interesting information in relevant class members 

	for(_cur_node = root->children; _cur_node != NULL; _cur_node = _cur_node->next)
	{
		if ( _cur_node->type == XML_ELEMENT_NODE  &&
				!xmlStrcmp(_cur_node->name, (const xmlChar *) "NeuronTypes" ) )
		{
			xmlChar* temp1=xmlGetProp(_cur_node,(const xmlChar*)"totalNeurons");
			_num_neurons=atoi( (char*) temp1);
			xmlChar* temp2=xmlGetProp(_cur_node,(const xmlChar*)"miniColumns");
			_num_mini= atoi( (char*) temp2);

			for(_child_node=_cur_node->children;_child_node!=NULL;_child_node=_child_node->next)
			{
				if ( _cur_node->type == XML_ELEMENT_NODE  &&  !xmlStrcmp(_child_node->name, (const xmlChar *)"Layer") )
				{
					_num_layers++;	
					for(_child_node2=_child_node->children;_child_node2!=NULL;_child_node2=_child_node2->next)
					{
						if ( _child_node->type == XML_ELEMENT_NODE  &&  !xmlStrcmp(_child_node2->name, (const xmlChar *)"StructuralType") )
						{
							xmlChar* temp3=xmlGetProp(_child_node2,(const xmlChar*)"id");
							std::string s( (char*) temp3);
							_morph_set.insert(s);
							xmlFree(temp3);
						}
					}
				}
			}
			xmlFree(temp1);
			xmlFree(temp2);
		}
	}
	//
	//NOW PARSE CONNECTION PROBABILITIES FILE
	//	
	_doc2= xmlParseFile(conn_path.c_str());
	if (_doc2 == NULL) 
		printf("error: could not parse file \n");

	/*Get the root element node */
	xmlNode *root2 = NULL;
	root2 = xmlDocGetRootElement(_doc2);

	// Must have root element, a name and the name must be "blueColumn"

	if( !root2 || 
			!root2->name ||
			xmlStrcmp(root2->name,(const xmlChar*)"ConnectionRules") ) 
	{
		xmlFreeDoc(_doc2);
	}
	// This for loop parses the xml doc and stores the interesting information in relevant class members 

	for(_cur_node = root2->children; _cur_node != NULL; _cur_node = _cur_node->next)
	{
		//mtype info
		if ( _cur_node->type == XML_ELEMENT_NODE  &&
				!xmlStrcmp(_cur_node->name, (const xmlChar *) "mTypeRule" ) )
		{
			xmlChar* temp1=xmlGetProp(_cur_node,(const xmlChar*)"from");
			xmlChar* temp2=xmlGetProp(_cur_node,(const xmlChar*)"to");
			xmlChar* temp3=xmlGetProp(_cur_node,(const xmlChar*)"probability");

			_mtype_connectivity[std::string((const char*)temp1)][std::string((const char*)temp2)] = atoi( (const char*)temp3 );
			xmlFree(temp1);
			xmlFree(temp2);
			xmlFree(temp3);
		}

		//layer info
		if ( _cur_node->type == XML_ELEMENT_NODE  &&
				!xmlStrcmp(_cur_node->name, (const xmlChar *) "layerRule" ) )
		{
			xmlChar* temp1=xmlGetProp(_cur_node,(const xmlChar*)"from");
			xmlChar* temp2=xmlGetProp(_cur_node,(const xmlChar*)"to");
			xmlChar* temp3=xmlGetProp(_cur_node,(const xmlChar*)"probability");

			_layer_connectivity[atoi((const char*)temp1)][atoi((const char*)temp2)] = atoi( (const char*)temp3 );
			xmlFree(temp1);
			xmlFree(temp2);
			xmlFree(temp3);
		}

		//neuron type(exc-inh) info
		if ( _cur_node->type == XML_ELEMENT_NODE  &&
				!xmlStrcmp(_cur_node->name, (const xmlChar *) "sClassRule" ) )
		{
			xmlChar* temp1=xmlGetProp(_cur_node,(const xmlChar*)"from");
			xmlChar* temp2=xmlGetProp(_cur_node,(const xmlChar*)"to");
			xmlChar* temp3=xmlGetProp(_cur_node,(const xmlChar*)"probability");

			_sclass_connectivity[std::string((const char*)temp1)][std::string((const char*)temp2)] = atoi( (const char*)temp3 );
			xmlFree(temp1);
			xmlFree(temp2);
			xmlFree(temp3);
		}


	}


}

// ----------------------------------------------------------------------------------------
inline
Recipe_XML_File_Parser::~Recipe_XML_File_Parser()
{
	/*free the document */
	xmlFreeDoc(_doc);
	xmlFreeDoc(_doc2);
}

// ----------------------------------------------------------------------------------------

inline
Count Recipe_XML_File_Parser::number_of_neurons() const
{
	return _num_neurons;
}

// ----------------------------------------------------------------------------------------

inline
Count Recipe_XML_File_Parser::number_of_minicolumns() const
{
	return _num_mini;
}

// ----------------------------------------------------------------------------------------

inline
Count Recipe_XML_File_Parser::number_of_morphologies() const
{
	return _morph_set.size();
}

// ----------------------------------------------------------------------------------------

inline
Cortical_Layer_Number Recipe_XML_File_Parser::number_of_layers() const
{
	return _num_layers;
}

// ----------------------------------------------------------------------------------------

inline
std::set<std::string> Recipe_XML_File_Parser::all_morphologies() const
{
	return _morph_set;
}

// ----------------------------------------------------------------------------------------


inline
std::map<Cortical_Layer_Number, std::map<Cortical_Layer_Number,Probability> > Recipe_XML_File_Parser::layer_connection_probabilities() const
{
return _layer_connectivity;
}

// ----------------------------------------------------------------------------------------

inline
std::map<Label, std::map<Label,Probability> > Recipe_XML_File_Parser::morphology_connection_probabilities() const
{
return _mtype_connectivity;
}

// ----------------------------------------------------------------------------------------

inline
std::map< Label, std::map<Label, Probability> > Recipe_XML_File_Parser::neurontype_connection_probabilities() const
{
return _sclass_connectivity;
}
}//namespace bbp

#endif//for libxml
#endif//include guard


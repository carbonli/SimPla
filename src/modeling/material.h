/*
 * material.h
 *
 *  Created on: 2013年12月15日
 *      Author: salmon
 */

#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <algorithm>
#include <bitset>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../fetl/primitives.h"
#include "../utilities/log.h"
#include "../utilities/pretty_stream.h"
#include "pointinpolygen.h"
#include "select.h"
namespace simpla
{

template<typename TM>
class Material
{

public:
	static constexpr int MAX_NUM_OF_MEIDA_TYPE = 64;
	typedef TM mesh_type;

	typedef std::bitset<MAX_NUM_OF_MEIDA_TYPE> material_type;
	typedef typename mesh_type::index_type index_type;
	typedef typename mesh_type::coordinates_type coordinates_type;

	const material_type null_material;

	std::vector<material_type> material_[mesh_type::NUM_OF_COMPONENT_TYPE];
	std::map<std::string, material_type> register_material_;
	unsigned int max_material_;
	bool isChanged_;
public:

	enum
	{
		NONE = 0, VACUUM = 1, PLASMA, CORE, BOUNDARY, PLATEAU, LIMTER,
		// @NOTE: add materials for different physical area or media
		CUSTOM = 20
	};

	mesh_type const &mesh;

	Material(mesh_type const & m)
			: null_material(1 << NONE), mesh(m), max_material_(CUSTOM + 1), isChanged_(true)
	{
		register_material_.emplace("NONE", null_material);

		register_material_.emplace("Vacuum", material_type(1 << VACUUM));
		register_material_.emplace("Plasma", material_type(1 << PLASMA));
		register_material_.emplace("Core", material_type(1 << CORE));
		register_material_.emplace("Boundary", material_type(1 << BOUNDARY));
		register_material_.emplace("Plateau", material_type(1 << PLATEAU));
		register_material_.emplace("Limter", material_type(1 << LIMTER));

	}
	~Material()
	{
	}

	bool empty() const
	{
		return material_[VERTEX].empty();
	}

	operator bool() const
	{
		return material_[VERTEX].empty();
	}

	material_type RegisterMaterial(std::string const & name)
	{
		material_type res;
		if (register_material_.find(name) != register_material_.end())
		{
			res = register_material_[name];
		}
		else if (max_material_ < MAX_NUM_OF_MEIDA_TYPE)
		{
			res.set(max_material_);
			++max_material_;
		}
		else
		{
			ERROR << "Too much media Type";
		}
		return res;
	}

	unsigned int GetNumMaterialType() const
	{
		return max_material_;
	}
	material_type GetMaterialFromNumber(unsigned int material_pos) const
	{
		material_type res;
		res.set(material_pos);
		return std::move(res);
	}
	material_type GetMaterialFromString(std::string const &name) const
	{

		material_type res;

		try
		{
			res = register_material_.at(name);

		} catch (...)
		{
			ERROR << "Unknown material name : " << name;
		}
		return std::move(res);
	}
	material_type GetMaterialFromString(std::string const &name)
	{
		return std::move(RegisterMaterial(name));
	}

	std::vector<material_type> & operator[](unsigned int n)
	{

		return material_[n];
//		auto it = register_material_.find(name);
//		if (it != register_material_.end())
//		{
//			RegisterMaterial(name);
//		}
//
//		return std::move(register_material_.at(name));
	}

	std::vector<material_type> const& operator[](unsigned int n) const
	{
		return material_[n];
	}

	void ClearAll()
	{
		for (auto &v : material_[0])
		{
			v.reset();
		}

		Update();
	}

	template<typename TDict>
	void Load(TDict const & dict)
	{
		if (dict)
		{
			for (auto const & p : dict)
			{
				Modify(p.second);
			}
		}

	}
	template<typename OS>
	void Save(OS &os) const
	{
		UNIMPLEMENT;
//		os << "{ \n" << "\t -- register media type\n";
//
//		for (auto const& p : register_material_)
//		{
//			os << std::setw(10) << p.first << " = 0x" << std::hex << p.second.to_ulong() << std::dec << ", \n";
//		}
//
//		os << " }\n"
//
//		;
	}

	void Init(int I = VERTEX)
	{
		size_t num = mesh.GetNumOfElements(I);

		if (material_[I].size() < num)
		{
			material_[I].resize(num, null_material);
		}
	}

	template<typename TCmd>
	void Modify(TCmd const& cmd)
	{
		std::string op = "";
		std::string type = "";

		cmd["Op"].template as<std::string>(&op);
		cmd["Type"].template as<std::string>(&type);

		if (type == "")
		{
			WARNING << "Illegal input! [ undefine type ]";
			return;
		}

		auto select = cmd["Select"];
		if (select.empty())
		{
			std::vector<coordinates_type> Range;

			cmd["Range"].as(&Range);

			if (op == "Set")
			{
				Set(type, Range);
			}
			else if (op == "Remove")
			{
				Remove(type, Range);
			}
			else if (op == "Add")
			{
				Add(type, Range);
			}
		}
		else
		{
			if (op == "Set")
			{
				Set(type, select);
			}
			else if (op == "Remove")
			{
				Remove(type, select);
			}
			else if (op == "Add")
			{
				Add(type, select);
			}
		}

		LOGGER << op << " material " << type << DONE;
		isChanged_ = true;
	}

	template<typename ...Args> inline
	void Set(std::string material, Args const & ... args)
	{
		Set(GetMaterialFromString(material), std::forward<Args const &>(args)...);
	}
	template<typename ...Args> inline
	void Set(unsigned int material, Args const & ... args)
	{
		Set(GetMaterialFromNumber(material), std::forward<Args const &>(args)...);
	}

	template<typename ...Args> inline
	void Add(std::string material, Args const & ... args)
	{
		Add(GetMaterialFromString(material), std::forward<Args const &>(args)...);
	}
	template<typename ...Args> inline
	void Add(unsigned int material, Args const & ... args)
	{
		Add(GetMaterialFromNumber(material), std::forward<Args const &>(args)...);
	}

	template<typename ...Args> inline
	void Remove(std::string material, Args const & ... args)
	{
		Set(~GetMaterialFromString(material), std::forward<Args const &>(args)...);
	}
	template<typename ...Args> inline
	void Remove(unsigned int material, Args const & ... args)
	{
		Set(~GetMaterialFromNumber(material), std::forward<Args const &>(args)...);
	}

	/**
	 * Set material on vertics
	 * @param material is  set to 1<<material
	 * @param args args are trans-forward to
	 *      SelectVerticsInRange(<lambda function>,mesh,args)
	 */
	template<typename ...Args>
	void Set(material_type material, Args const & ... args)
	{
		_ForEachVertics([&]( material_type &v)
		{	v=material;},

		std::forward<Args const &>(args)...);
	}

	template<typename ...Args>
	void Add(material_type material, Args const & ... args)
	{

		_ForEachVertics([&]( material_type &v)
		{	v|=material;},

		std::forward<Args const &>(args)...);
	}

	template<typename ...Args>
	void Remove(material_type material, Args const & ... args)
	{

		_ForEachVertics([&]( material_type &v)
		{	v^=material;},

		std::forward<Args const &>(args)...);
	}

	/**
	 *  Update material on edge ,face and cell, base on material on vertics
	 */
	void Update()
	{
		Init(VERTEX);
		if (isChanged_)
		{
			_UpdateMaterials<EDGE>();
			_UpdateMaterials<FACE>();
			_UpdateMaterials<VOLUME>();
			isChanged_ = false;
		}
	}
	bool IsChanged() const
	{
		return isChanged_;
	}

	/**
	 *  Choice elements that most close to and out of the interface,
	 *  No element cross interface.
	 * @param
	 * @param fun
	 * @param in_material
	 * @param out_material
	 * @param flag
	 */
	typedef typename mesh_type::iterator iterator;

	Range<IteratorFilter<iterator>> SelectBoundary(iterator ib, iterator ie, material_type in, material_type out) const;

	Range<IteratorFilter<iterator>> SelectBoundary(iterator ib, iterator ie, std::string const & in,
	        std::string const & out) const
	{
		return SelectBoundary(ib, ie, GetMaterialFromString(in), GetMaterialFromString(out));
	}

	Range<IteratorFilter<iterator>> SelectCell(iterator ib, iterator ie, material_type) const;

	Range<IteratorFilter<iterator>> SelectCell(iterator ib, iterator ie, std::string const & m) const
	{
		return SelectCell(ib, ie, GetMaterialFromString(m));
	}

	template<typename TDict>
	Range<IteratorFilter<iterator>> Select(iterator ib, iterator ie, TDict const & dict) const;

private:

	/**
	 * Set material on vertics
	 * @param material is  set to 1<<material
	 * @param args args are trans-forward to
	 *      SelectVerticsInRange(<lambda function>,mesh,args)
	 */
	template<typename ...Args>
	void _ForEachVertics(std::function<void(material_type&)> fun, Args const & ... args)
	{
		Init();

		for (auto s : Filter(mesh.begin(VERTEX), mesh.end(VERTEX), std::forward<Args const&>(args)...))
		{
			fun(material_[VERTEX].at(mesh.Hash(s)));
		}
	}

	template<int IFORM>
	void _UpdateMaterials()
	{
		LOGGER << "Update Material " << IFORM;

		Init(IFORM);

		try
		{
			for (auto s : mesh.GetRange(IFORM))
			{

				index_type v[mesh_type::MAX_NUM_VERTEX_PER_CEL];

				int n = mesh.template GetAdjacentCells(Int2Type<IFORM>(), Int2Type<VERTEX>(), s, v);

				material_type flag = null_material;
				for (int i = 0; i < n; ++i)
				{
					flag |= material_[VERTEX].at(mesh.Hash(v[i]));
				}
				material_[IFORM].at(mesh.Hash(s)) = flag;

			}
		} catch (std::out_of_range const &e)
		{
			ERROR << " I = " << IFORM << std::endl

			<< e.what();
		}
	}
};
template<typename TM>
inline std::ostream & operator<<(std::ostream & os, Material<TM> const &self)
{
	self.Save(os);
	return os;
}

template<typename TM>
Range<IteratorFilter<typename TM::iterator>> Material<TM>::SelectBoundary(typename TM::iterator ib,
        typename TM::iterator ie, material_type in, material_type out) const
{
	if (IsChanged())
	{
		ERROR << "need update!!";
	}

	// Good
	//  +----------#----------+
	//  |          #          |
	//  |    A     #-> B   C  |
	//  |          #          |
	//  +----------#----------+
	//
	//  +--------------------+
	//  |         ^          |
	//  |       B |     C    |
	//  |     ########       |
	//  |     #      #       |
	//  |     #  A   #       |
	//  |     #      #       |
	//  |     ########       |
	//  +--------------------+
	//
	//             +----------+
	//             |      C   |
	//  +----------######     |
	//  |          | A  #     |
	//  |    A     | &  #  B  |
	//  |          | B  #->   |
	//  +----------######     |
	//             |          |
	//             +----------+
	//
	//     	       +----------+
	//       C     |          |
	//  +----------#----+     |
	//  |          # A  |     |
	//  |    B   <-# &  |  A  |
	//  |          # B  |     |
	//  +----------#----+     |
	//             |          |
	//             +----------+
	//
	//
	// 	 Bad
	//
	//  +--------------------+
	//  |                    |
	//  |        A           |
	//  |     ########       |
	//  |     #      #       |
	//  |     #->B C #       |
	//  |     #      #       |
	//  |     ########       |
	//  +--------------------+
	//
	// 	            +----------+
	//              |          |
	//   +-------+  |          |
	//   |       |  |          |
	//   |   B   |  |    A     |
	//   |       |  |          |
	//   +-------+  |          |
	//              |          |
	//              +----------+
	int IFORM = mesh._IForm(*ib);
	Range<IteratorFilter<typename TM::iterator>> res;

//	res = Filter(ib, ie,
//
//	[&]( index_type s, index_type *c)->int
//	{
//		int count=0;
//		if ((this->material_[IFORM].at(mesh.Hash(s)) & in).none()
//				&& (this->material_[IFORM].at(mesh.Hash(s)) & out).any())
//		{
//			index_type neighbours[mesh_type::MAX_NUM_NEIGHBOUR_ELEMENT];
//
//			int num = this->mesh.GetAdjacentCells(Int2Type<IFORM>(), Int2Type<VOLUME>(), s, neighbours);
//
//			for (int i = 0; i < num; ++i)
//			{
//				if (((this->material_[VOLUME].at(mesh.Hash(neighbours[i])) & in)).any())
//				{
//					c[count]=neighbours[i];
//					++count;
//				}
//			}
//		}
//
//		return count;
//	});

	return res;

}

template<typename TM>
Range<IteratorFilter<typename TM::iterator>> Material<TM>::SelectCell(typename TM::iterator ib,
        typename TM::iterator ie, material_type material) const
{
	return Filter(ib, ie, [&]( typename TM::iterator it, typename TM::index_type *c)->int
	{	c[0]=*it;
		return (((this->material_[mesh._IForm(*ib)].at(mesh.Hash(*it)) & material)).any())?1:0;
	});

}

template<typename TM>
template<typename TDict>
Range<IteratorFilter<typename TM::iterator>> Material<TM>::Select(typename TM::iterator ib, typename TM::iterator ie,
        TDict const & dict) const
{
	Range<IteratorFilter<typename TM::iterator>> res(ie, ie);
	if (dict["Type"])
	{
		auto type = dict["Type"].template as<std::string>("");

		if (type == "Boundary")
		{
			auto material = GetMaterialFromString(dict["Material"].template as<std::string>());
			res = SelectBoundary(ib, ie, material, null_material);

		}
		else if (type == "Interface")
		{
			auto in = GetMaterialFromString(dict["In"].template as<std::string>());
			auto out = GetMaterialFromString(dict["Out"].template as<std::string>());
			res = SelectBoundary(ib, ie, in, out);
		}
		else if (type == "Element")
		{
			auto material = GetMaterialFromString(dict["Material"].template as<std::string>());
			res = SelectCell(ib, ie, material);
		}
	}

	return res;

}

}
// namespace simpla

#endif /* MATERIAL_H_ */

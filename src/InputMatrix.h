#pragma once

#include "Input.h"
#include "CommandEntry.h"

namespace DIMS
{


	//Move to type aliases maybe?
	using CommandMap = std::unordered_map<Input::Hash, std::vector<std::shared_ptr<CommandEntry>>>;



	struct Matrix
	{
		struct Instance
		{
			virtual ~Instance() = default;

			Matrix* base = nullptr;

			RE::InputContextID context = RE::InputContextID::kNone;

			CommandMap storage;

			//Utilize covariance with this
			Matrix* GetBaseObject()
			{
				return base;
			}
			const Matrix* GetBaseObject() const
			{
				return base;
			}

			//Ensure that an IMatrix makes this.
		};
		using InstancePtr = std::unique_ptr<Instance>;




		virtual ~Matrix() = default;



		virtual bool CanInputPass(RE::InputEvent* event) const
		{
			return true;
		}


		virtual MatrixType GetMatrixType() const = 0;


		//This can be overriden with a custom entry, such as for states.
		virtual Instance* GetContextInstance(RE::InputContextID context, bool create_if_required = false) = 0;

		virtual void DestroyInstance(RE::InputContextID context) = 0;


		virtual std::strong_ordering CompareOrder(const Matrix* other) const
		{
			return std::strong_ordering::equal;
		}


		virtual CommandMap CreateMap() = 0;


		template <class Self>
		auto ObtainContextInstance(this Self&& a_this, RE::InputContextID context)// -> Self::Instance*
		{
			using Instance = std::remove_pointer_t<std::remove_cvref_t<Self>>::Instance;
			return static_cast<Instance*>(a_this.GetContextInstance(context, true));
		}
	};


	struct InputMatrix : public Matrix
	{

		std::map<RE::InputContextID, InstancePtr> entries;

		std::vector<InputCommand> commands;//Once this is finalized, this cannot have it's values changed. so it should be private.

		MatrixType type = MatrixType::Total;




	protected:


		template<std::derived_from<Instance> IType>
		IType* LoadInstance(RE::InputContextID context, bool create_if_required = false)
		{
			//The idea with this is making a setup where the item pocket, and the thing gotten are one in the same.
			auto& slot = entries[context];

			if (create_if_required && !slot) {
				slot = std::make_unique<IType>();
				slot->base = this;
				slot->context = context;
				slot->storage = CreateMap();
				//TODO: Need to make context right here.
			}

			return static_cast<IType*>(slot.get());
		}
	public:

		Instance* GetContextInstance(RE::InputContextID context, bool create_if_required = false) override
		{
			return LoadInstance<Instance>(context, create_if_required);
		}

		void DestroyInstance(RE::InputContextID context) override
		{
			entries.erase(context);
		}

		MatrixType GetMatrixType() const override
		{
			return type;
		}


		MatrixType FetchMatrixType() const
		{
			return this ? GetMatrixType() : MatrixType::Dynamic;
		}

		CommandMap CreateMap() override
		{
			CommandMap map;

			for (auto& command : commands)
			{
				for (auto& trigger : command.triggers)
				{
					CommandEntryPtr entry = std::make_shared<CommandEntry>(&command, &trigger);

					for (auto input : trigger.GetInputs())
					{
						auto& list = map[input];

						list.insert(std::upper_bound(list.begin(), list.end(), entry, [](const std::shared_ptr<CommandEntry>& lhs, const std::shared_ptr<CommandEntry>& rhs)
							{return lhs->priority() > rhs->priority(); }),
							entry);
					}
				}
			}

			return map;
		}

	};
}
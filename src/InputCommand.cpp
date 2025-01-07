#include "InputCommand.h"

#include "InputMatrix.h"

namespace DIMS
{

	MatrixType InputCommand::GetParentType() const
	{
		return parent->FetchMatrixType();
	}


	std::strong_ordering InputCommand::CompareOrder(const InputCommand* other) const
	{
		switch (strong_ordering_to_int(GetParentType() <=> other->GetParentType()))
		{
			//This is reversed, as 0 is considered higher than 1 and so on.
		case  1:
			return std::strong_ordering::less;
		case -1:
			return std::strong_ordering::greater;

		case  0:
			break;
		}



		if (parent && other->parent)
		{
			switch (strong_ordering_to_int(parent->CompareOrder(other->parent)))
			{
				//This is reversed, as 0 is considered higher than 1 and so on.
			case  1:
				return std::strong_ordering::greater;
			case -1:
				return std::strong_ordering::less;

			case  0:
				break;
			}
		}


		return std::strong_ordering::equal;
	}

}
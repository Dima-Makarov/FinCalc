#include "fin_calc.h"
#include "stdexcept"

LongFloat::LongFloat() {
	whole_part_.resize(13);
	fractional_part_.resize(6);

}

LongFloat::LongFloat(std::string str)
{	
	std::remove(str.begin(), str.end(), ' ');
	if (str.size() == 0) {
		throw std::exception("Empty number");
	}
	for (auto s : str) {
		if (!(s >= '0' && s <= '9') && s != '.' && s != ',' && s != '-') {
			std::string msg = "Wrong symbol found: ";
			msg.push_back(s);
			throw std::exception(msg.c_str());
		}
	}
	for (int i = 1; i < str.size(); i++) {
		if (str[i] == '-') {
			throw std::exception("Minus is misplaced");
		}
	}
	if (str[0] == '-') {
		is_negative_ = true;
		str.erase(str.begin());
	}
	bool got_dot = false;
	bool got_comma = false;
	int separator_index = str.size();
	for (auto i = 0; i < str.size(); i++) {
		if (str[i] == '.') {
			if (got_dot || got_comma) {
				throw std::exception("Excess amount of dots/commas");
			}
			got_dot = true;
			separator_index = i;
		}
		if (str[i] == ',') {
			if (got_dot || got_comma) {
				throw std::exception("Excess amount of dots/commas");
			}
			got_comma = true;
			separator_index = i;
		}
	}
	for (int i = 0; i < separator_index; i++) {
		whole_part_.push_back(str[i] - '0');
	}
	for (int i = separator_index + 1; i < str.size(); i++) {
		fractional_part_.push_back(str[i] - '0');
	}
}

std::string LongFloat::to_string() const
{	
	LongFloat a = *this;
	a = Normalize(a);
	std::string result;
	bool got_non_zero = false;
	for (int i = 0; i < a.whole_part_.size(); i++) {
		if (a.whole_part_[i] > 0) {
			got_non_zero = true;
		} else if(!got_non_zero) {
			continue;
		}
		result.append(std::to_string(a.whole_part_[i]));
	}
	
	int counter = 1;
	for (int i = result.size() - 1; i >= 0; i--, counter++) {
		if (counter % 3 == 0) {
			result.insert(i, " ");
		}
	}
	result.append(".");
	for (int i = 0; i < a.fractional_part_.size(); i++) {
		result.append(std::to_string(a.fractional_part_[i]));
	}
	if (result[0] == '.') {
		result.insert(result.begin(), '0');
	}
	if (a.is_negative_) {
		result.insert(result.begin(), '-');
	}
	return result;
}

LongFloat LongFloat::operator+(const LongFloat& second)
{
	LongFloat a = Normalize(*this);
	LongFloat b = Normalize(second);
	LongFloat result;
	if (!a.is_negative_ && !b.is_negative_) {
		result = SumTwoNumbersWnoSign(a, b);
	}
	if (a.is_negative_ && !b.is_negative_) {
		a.is_negative_ = false;
		result = b - a;
	}
	if (!a.is_negative_ && b.is_negative_) {
		b.is_negative_ = false;
		result = a - b;
	}
	if (a.is_negative_ && b.is_negative_) {
		result = SumTwoNumbersWnoSign(a, b);
		result.is_negative_ = true;
	}

	return result;
}

LongFloat LongFloat::operator-(const LongFloat& second)
{
	LongFloat a = Normalize(*this);
	LongFloat b = Normalize(second);
	LongFloat result;
	if (a == b) {
		return LongFloat("0.0");
	}
	if (!a.is_negative_ && !b.is_negative_ && a > b) {
		result = SubTwoNumbersWnoSign(a, b);
	}
	if (!a.is_negative_ && !b.is_negative_ && a < b) {
		result = SubTwoNumbersWnoSign(b,a);
		result.is_negative_ = true;
	}
	if (!a.is_negative_ && b.is_negative_) {
		result = SumTwoNumbersWnoSign(a, b);
	}
	if (a.is_negative_ && !b.is_negative_) {
		result = SumTwoNumbersWnoSign(a, b);
		result.is_negative_ = true;
	}
	if (a.is_negative_ && b.is_negative_ && a > b) {
		result = SubTwoNumbersWnoSign(b, a);
	}
	if (a.is_negative_ && b.is_negative_ && a < b) {
		a.is_negative_ = false;
		result = SubTwoNumbersWnoSign(a, b);
		result.is_negative_ = true;
	}
	return result;
}

bool LongFloat::operator==(const LongFloat& second) const
{	
	LongFloat a = Normalize(*this);
	LongFloat b = Normalize(second);
	if (this->is_negative_ != second.is_negative_) {
		return false;
	}
	for (int i = 0; i < a.whole_part_.size(); i++) {
		if (a.whole_part_[i] != b.whole_part_[i]) {
			return false;
		}
	}
	for (int i = 0; i < a.fractional_part_.size(); i++) {
		if (a.fractional_part_[i] != b.fractional_part_[i]) {
			return false;
		}
	}
	return true;
}

bool LongFloat::operator!=(const LongFloat& second) const
{
	return !(*this == second);
}

bool LongFloat::operator>(const LongFloat& second) const
{
	LongFloat a = Normalize(*this);
	LongFloat b = Normalize(second);
	if (a.is_negative_ && !b.is_negative_) {
		return false;
	}
	if (!a.is_negative_ && b.is_negative_) {
		return true;
	}
	if (a.is_negative_ && b.is_negative_) {
		for (int i = 0; i < a.whole_part_.size(); i++) {
			if (a.whole_part_[i] < b.whole_part_[i]) {
				return true;
			}
			if (a.whole_part_[i] > b.whole_part_[i]) {
				return false;
			}
		}
		for (int i = 0; i < a.fractional_part_.size(); i++) {
			if (a.fractional_part_[i] < b.fractional_part_[i]) {
				return true;
			}
			if (a.fractional_part_[i] > b.fractional_part_[i]) {
				return false;
			}
		}
	}
	if (!a.is_negative_ && !b.is_negative_) {
		for (int i = 0; i < a.whole_part_.size(); i++) {
			if (a.whole_part_[i] > b.whole_part_[i]) {
				return true;
			}
			if (a.whole_part_[i] < b.whole_part_[i]) {
				return false;
			}
		}
		for (int i = 0; i < a.fractional_part_.size(); i++) {
			if (a.fractional_part_[i] > b.fractional_part_[i]) {
				return true;
			}
			if (a.whole_part_[i] < b.whole_part_[i]) {
				return false;
			}
		}
	}
	return false;
}

bool LongFloat::operator<(const LongFloat& second) const
{
	return !(*this > second) && !(*this == second);
}

LongFloat LongFloat::Normalize(const LongFloat& f)
{
	LongFloat result = f;
	result.fractional_part_.resize(6);
	int size = result.whole_part_.size();
	if (size > 13) {
		throw std::exception("Number too big");
	}
	if (size < 13) {
		for (int i = 0; i < 13 - size; i++) {
			result.whole_part_.insert(result.whole_part_.begin(), 0);
		}
	}
	return result;
}

LongFloat LongFloat::SumTwoNumbersWnoSign(const LongFloat& one, const LongFloat& two)
{
	LongFloat result;
	int carry = 0;
	for (int i = one.fractional_part_.size() - 1; i >= 0; i--) {
		result.fractional_part_[i] = one.fractional_part_[i] + two.fractional_part_[i] + carry;
		carry = 0;
		if (result.fractional_part_[i] > 9) {
			carry = 1;
			result.fractional_part_[i] -= 10;
		}
	}
	for (int i = one.whole_part_.size() - 1; i >= 0; i--) {
		result.whole_part_[i] = one.whole_part_[i] + two.whole_part_[i] + carry;
		carry = 0;
		if (result.whole_part_[i] > 9) {
			carry = 1;
			result.whole_part_[i] -= 10;
		}
	}
	if (carry != 0) {
		throw std::exception("Overflow");
	}
	return result;
}

LongFloat LongFloat::SubTwoNumbersWnoSign(const LongFloat& one, const LongFloat& two)
{
	LongFloat result;
	int borrow = 0;
	for (int i = one.fractional_part_.size() - 1; i >= 0; i--) {
		int diff = one.fractional_part_[i] - two.fractional_part_[i] - borrow;
		if (diff < 0) {
			diff += 10;
			borrow = 1;
		}
		else {
			borrow = 0;
		}
		result.fractional_part_[i] = diff;
	}
	for (int i = one.whole_part_.size() - 1; i >= 0; i--) {
		int diff = one.whole_part_[i] - two.whole_part_[i] - borrow;
		if (diff < 0) {
			diff += 10;
			borrow = 1;
		}
		else {
			borrow = 0;
		}
		result.whole_part_[i] = diff;
	}
	return result;
}

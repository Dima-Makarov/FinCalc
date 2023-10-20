#include "fin_calc.h"
#include "stdexcept"
#include <sstream>
#include <iomanip>
#include "big_int.hpp"

LongFloat::LongFloat() {
	whole_part_.resize(13);
	fractional_part_.resize(6);
}

LongFloat::LongFloat(std::string str)
{	
	bool contains_only_spaces = true;
	for (auto i : str) {
		if (i != ' ') {
			contains_only_spaces = false;
		}
	}
	if (contains_only_spaces || str.size() == 0) {
		throw std::invalid_argument(u8"Пустое число");
	}
	for (auto s : str) {
		if (!(s >= '0' && s <= '9') && s != '.' && s != ',' && s != '-' && s != ' ') {
			std::string msg = u8"Обнаружен неправильный символ: ";
			msg.push_back(s);
			throw std::invalid_argument(msg);
		}
	}
	for (int i = 0; i < str.size(); i++) {
		if (str[i] != ' ') {
			break;
		}
		else {
			str.erase(str.begin());
			i--;
		}
	}
	for (int i = 1; i < str.size(); i++) {
		if (str[i] == '-') {
			throw std::invalid_argument(u8"Минус не на своем месте");
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
				throw std::invalid_argument(u8"Слишком много точек/запятых");
			}
			got_dot = true;
			separator_index = i;
		}
		if (str[i] == ',') {
			if (got_dot || got_comma) {
				throw std::invalid_argument(u8"Слишком много точек/запятых");
			}
			got_comma = true;
			separator_index = i;
		}
	}
	for (int i = separator_index-1; i >= 0; i--) {
		if ((separator_index - i) % 4 == 0) {
			continue;
		}
		if (str[i] == ' ') {
			throw std::invalid_argument(u8"Неправильное положение пробела");
		}
	}
	for (int i = separator_index-1; i < str.length(); i++) {
		if (str[i] == ' ') {
			throw std::invalid_argument(u8"Пробелы в дробной части недопустимы");
		}
	}
	for (int i = 0; i < str.length(); i++) {
		if (str[i] == ' ') {
			continue;
		}
		if (str[i] == '.' || str[i] == ',') {
			break;
		}
		else {
			whole_part_.push_back(str[i] - '0');
		}
	}
	for (int i = separator_index + 1; i < str.size(); i++) {
		fractional_part_.push_back(str[i] - '0');
	}
}

LongFloat::LongFloat(long double val)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(std::numeric_limits<long double>::digits10 + 1) << val;


	*this = LongFloat(ss.str());
}

std::string LongFloat::to_string() const
{	
	LongFloat a = *this;
	a = Normalize(a);
	std::string result;
	if (a == LongFloat("0,0")) {
		return "0";
	}
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
	bool fract_all_zero = true;
	for (int i = 0; i < a.fractional_part_.size(); i++) {
		if (a.fractional_part_[i] != 0) {
			fract_all_zero = false;
		}
	}
	if (!fract_all_zero) {
		result.append(".");
	}
	for (int i = a.fractional_part_.size() - 1; i >= 0; i--) {
		if (a.fractional_part_[i] == 0) {
			a.fractional_part_.pop_back();
		} else {
			break;
		}
	}
	for (int i = 0; i < a.fractional_part_.size(); i++) {
		result.append(std::to_string(a.fractional_part_[i]));
	}
	if (result[0] == '.') {
		result.insert(result.begin(), '0');
	}
	if (a.is_negative_) {
		result.insert(result.begin(), '-');
	}
	if (result[0] == '-') {
		if (result[1] == ' ') {
			result.erase(1, 2);
		}
	}
	if (result[0] == ' ') {
		result.erase(0, 1);
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

LongFloat LongFloat::operator*(const LongFloat& second)
{
	LongFloat num_a = Normalize(*this);
	LongFloat num_b = Normalize(second);
	

	int total_length = num_a.fractional_part_.size() + num_a.whole_part_.size();
	std::vector<int8_t> a(total_length);
	std::vector<int8_t> b(total_length);
	for (int i = 0; i < num_a.whole_part_.size(); i++) {
		a[i] = num_a.whole_part_[i];
		b[i] = num_b.whole_part_[i];
	}
	for (int i = num_a.whole_part_.size(); i < total_length; i++) {
		a[i] = num_a.fractional_part_[i - num_a.whole_part_.size()];
		b[i] = num_b.fractional_part_[i - num_a.whole_part_.size()];
	}
	std::vector<int8_t> c(total_length * 2 + 1);
	std::reverse(a.begin(), a.end());
	std::reverse(b.begin(), b.end());

	for (int i = 0; i < total_length; i++) {
		for (int j = 0; j < total_length; j++) {
			c[i + j] += a[i] * b[j];
			c[i + j + 1] = c[i + j + 1] + c[i + j] / 10;
			c[i + j] %= 10;
		}
	}

	std::reverse(c.begin(), c.end());

	LongFloat result;
	for (int i = c.size() - 12; i < c.size() - 6; i++) {
		result.fractional_part_[i - c.size() + 12] = c[i];
	}

	int first_nonzero_index = c.size() - 12;

	for (int i = 0; i < c.size() - 12; i++) {
		if (c[i] != 0) {
			first_nonzero_index = i;
			break;
		}
	}

	if (c.size() - first_nonzero_index - 12 > 13) {
		throw std::invalid_argument(u8"Число слишком большое");
	}
	
	int begin_index = c.size() - 12 - 13;

	for (int i = begin_index; i < c.size() - 12; i++) {
		result.whole_part_[i - begin_index] = c[i];
	}

	result.is_negative_ = num_a.is_negative_ ^ num_b.is_negative_;

	return result;
}

LongFloat LongFloat::operator/(const LongFloat& second)
{
	LongFloat num_a = Normalize(*this);
	LongFloat num_b = Normalize(second);
	if (num_b == LongFloat("0")) {
		throw std::invalid_argument(u8"Деление на ноль");
	}
	auto vec_to_str = [](const std::vector<int8_t>& vec) {
		std::string result;
		for (auto i : vec) {
			result.push_back(static_cast<char>(i + '0'));
		}
		return result;
	};
	int total_length = num_a.fractional_part_.size() + num_a.whole_part_.size();
	std::vector<int8_t> a(total_length);
	std::vector<int8_t> b(total_length);
	for (int i = 0; i < num_a.whole_part_.size(); i++) {
		a[i] = num_a.whole_part_[i];
		b[i] = num_b.whole_part_[i];
	}
	for (int i = num_a.whole_part_.size(); i < total_length; i++) {
		a[i] = num_a.fractional_part_[i - num_a.whole_part_.size()];
		b[i] = num_b.fractional_part_[i - num_a.whole_part_.size()];
	}

	bi::big_int a_lib;
	bi::big_int b_lib;

	a_lib.big_int_from_string(vec_to_str(a), bi::bi_base::BI_DEC);
	b_lib.big_int_from_string(vec_to_str(b), bi::bi_base::BI_DEC);
	bi::big_int quotient, remainder;
	a_lib.big_int_div(b_lib, quotient, remainder);
	std::string q_debug = quotient.big_int_to_string(bi::bi_base::BI_DEC);
	std::string r_debug = remainder.big_int_to_string(bi::bi_base::BI_DEC);

	auto rem_str = remainder.big_int_to_string(bi::bi_base::BI_DEC);
	long double rem_double = std::stoll(rem_str);

	rem_double /= 1e6;
	rem_double /= num_b.ToLongDouble();
	rem_double *= 1e6;
	int64_t rem_int = static_cast<int64_t>(rem_double);
	std::stringstream ss;
	ss << std::setw(6) << std::setfill('0') << rem_int;
	LongFloat result = std::string(quotient.big_int_to_string(bi::bi_base::BI_DEC)
		+ "."
		+ std::string(ss.str()));
	result.is_negative_ = num_a.is_negative_ ^ num_b.is_negative_;

	return result;
}

long double LongFloat::ToLongDouble() const
{
	LongFloat a = Normalize(*this);
	auto powdrob = [](long double a, int p) {
		long double result = 1;
		for (int i = 0; i < p; i++) {
			result *= a;
		}
		return result;
	};
	long double result = 0;
	std::reverse(a.whole_part_.begin(), a.whole_part_.end());
	for (int i = 0; i < a.whole_part_.size(); i++) {
		result += pow(10, i) * a.whole_part_[i];
	}
	for (int i = 0; i < a.fractional_part_.size(); i++) {
		result += pow(10, -i - 1) * a.fractional_part_[i];
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
		throw std::invalid_argument(u8"Число слишком большое");
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
		throw std::invalid_argument(u8"Число слишком большое");
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

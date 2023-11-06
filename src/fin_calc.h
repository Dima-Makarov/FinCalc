#pragma once

#include <vector>
#include <string>

class LongFloat {
public:
	LongFloat();
	LongFloat(std::string str);
	LongFloat(long double val);
	std::string to_string() const;
	LongFloat operator+(const LongFloat& second) const;
	LongFloat operator-(const LongFloat& second) const;
	LongFloat operator*(const LongFloat& second) const;
	LongFloat operator/(const LongFloat& second) const;
	long double ToLongDouble() const;
	void MathRound();
	void BankRound();
	void TruncRound();
	LongFloat GetFractionalPart() const;
	LongFloat GetWholePart() const;

	bool operator==(const LongFloat& second)const;
	bool operator!=(const LongFloat& second)const;
	bool operator>(const LongFloat& second)const;
	bool operator<(const LongFloat& second)const;


private:
	static LongFloat Normalize(const LongFloat& f);
	static LongFloat SumTwoNumbersWnoSign(const LongFloat& one, const LongFloat& two);
	static LongFloat SubTwoNumbersWnoSign(const LongFloat& one, const LongFloat& two);
	static LongFloat RoundTo6Decimals(const LongFloat& a);
	bool is_negative_ = false;
	std::vector<int8_t> whole_part_;
	std::vector<int8_t> fractional_part_;
};

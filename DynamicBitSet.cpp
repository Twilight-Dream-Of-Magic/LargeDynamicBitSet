#include "DynamicBitSet.hpp"

namespace TwilightDream
{
	/* BooleanBitWrapper */

	BooleanBitWrapper::BooleanBitWrapper() : bits( 0 ) {}
	BooleanBitWrapper::BooleanBitWrapper( uint32_t value ) : bits( value ) {}
	BooleanBitWrapper::~BooleanBitWrapper()
	{
		bits = 0;
	}

	// 按位与操作
	void BooleanBitWrapper::bit_and( uint32_t other )
	{
		bits &= other;
	}

	// 按位或操作
	void BooleanBitWrapper::bit_or( uint32_t other )
	{
		bits |= other;
	}

	// 按位非操作
	void BooleanBitWrapper::bit_not()
	{
		bits = ~bits;
	}

	// 按位异或操作
	void BooleanBitWrapper::bit_xor( uint32_t other )
	{
		bits ^= other;
	}

	// 按位同或操作
	void BooleanBitWrapper::bit_not_xor( uint32_t other )
	{
		bits = ~( bits ^ other );
	}

	// 按位非与操作
	void BooleanBitWrapper::bit_not_and( uint32_t other )
	{
		bits = ~( bits & other );
	}

	// 按位非或操作
	void BooleanBitWrapper::bit_not_or( uint32_t other )
	{
		bits = ~( bits | other );
	}

	// 按位左移操作
	void BooleanBitWrapper::bit_leftshift( int shift )
	{
		bits <<= shift;
	}

	// 按位右移操作
	void BooleanBitWrapper::bit_rightshift( int shift )
	{
		bits >>= shift;
	}

	// 设置所有位为给定的布尔值
	void BooleanBitWrapper::bit_set( bool value )
	{
		bits = value ? 0xFFFFFFFF : 0;
	}

	// 设置指定索引的位为给定的布尔值
	void BooleanBitWrapper::bit_set( bool value, int index )
	{
		if ( value )
		{
			bits |= ( 1 << index );
		}
		else
		{
			bits &= ~( 1 << index );
		}
	}

	// 翻转指定索引的位
	void BooleanBitWrapper::bit_flip( size_t index )
	{
		bits ^= ( 1 << index );
	}

	// 获取指定索引的位的布尔值
	bool BooleanBitWrapper::bit_get( int index ) const
	{
		return ( bits >> index ) & 1;
	}

	// 统计比特'1'的数量
	size_t BooleanBitWrapper::count_bits() const
	{
		uint32_t n = bits;

		// 将相邻的位分组，每两位一组，然后用这两位中较低的一位表示这一组中置位的数量（0或1或2）
		// 例如: 0b1101 (原始数值) 变成 0b0100
		n = n - ( ( n >> 1 ) & 0x55555555 );

		// 将相邻的两组位（即4位）合并为一组，然后用这一组中较低的两位表示这一组中置位的数量（0到4）
		// 例如: 0b0100 (来自上一步) 变成 0b0010
		n = ( n & 0x33333333 ) + ( ( n >> 2 ) & 0x33333333 );

		// 将相邻的两组位（即8位）合并为一组，然后用这一组中较低的4位表示这一组中置位的数量（0到8）
		// 并且我们通过和 0x0F0F0F0F 相与，消除了不需要的位
		n = ( n + ( n >> 4 ) ) & 0x0F0F0F0F;

		// 将32位数中的所有8位组合并，得到一个8位数，这个8位数的低8位表示原32位数中置位的数量（0到32）
		n = n + ( n >> 8 );

		// 同上，但这次是将两个8位数合并为一个16位数
		n = n + ( n >> 16 );

		// 使用与操作消除不需要的位，返回计数结果
		return n & 0x3F;
	}

	BooleanBitWrapper BooleanBitWrapper::operator=( const BooleanBitWrapper& other )
	{
		if ( this == &other )
		{
			return *this;
		}

		bits = other.bits;
		return *this;
	}

	bool operator==( const BooleanBitWrapper& left, const BooleanBitWrapper& right )
	{
		return left.bits == right.bits;
	}

	bool operator!=( const BooleanBitWrapper& left, const BooleanBitWrapper& right )
	{
		return left.bits != right.bits;
	}

	/* DynamicBitSet::BitIteratorBase */

	DynamicBitSet::BitIteratorBaseData::BitIteratorBaseData(std::vector<BooleanBitWrapper>* container_pointer, size_t chunk_index, size_t bit_offset, size_t max_valid_bits)
		:
		container_pointer(container_pointer),
		chunk_index(chunk_index),
		is_end_iterator(false),
		bit_offset(bit_offset),
		max_valid_bits(max_valid_bits)
	{
		if(container_pointer->empty())
			throw std::logic_error("DynamicBitSet::BitIteratorBase::BitIteratorBaseData() A container of empty bit blocks does not allow bit iterators!");
		
		if (container_pointer && chunk_index < container_pointer->size())
			data_pointer = &(*container_pointer)[chunk_index];
	}

	DynamicBitSet::BitIteratorBaseData::BitIteratorBaseData(std::vector<BooleanBitWrapper>* container_pointer, size_t max_valid_bits)
		: 
		container_pointer(container_pointer),
		chunk_index(container_pointer->size()),
		is_end_iterator(true),
		data_pointer(nullptr),
		bit_offset(max_valid_bits % std::numeric_limits<std::uint32_t>::digits),
		max_valid_bits(max_valid_bits)
	{
		if(container_pointer->empty())
			throw std::logic_error("DynamicBitSet::BitIteratorBase::BitIteratorBaseData() A container of empty bit blocks does not allow bit iterators!");
	}

	DynamicBitSet::BitIteratorBaseData::BitIteratorBaseData(std::vector<BooleanBitWrapper>* container_pointer, size_t max_valid_bits, bool is_reverse_end_iterator)
		:
		DynamicBitSet::BitIteratorBaseData(container_pointer, max_valid_bits)
	{
		is_end_iterator = false; //对于正向迭代器来说，不关心反向迭代器的状态，只关心它的初始设置。
		
		if (is_reverse_end_iterator)
		{
			//立即设置反向迭代器结束位置附近
			chunk_index = 0;
			bit_offset = 0;
		}
		else
		{
			//立即设置反向迭代器开始位置
			chunk_index = container_pointer->size() - 1;
			bit_offset = (max_valid_bits - 1) % 32;
		}

		if (container_pointer && chunk_index < container_pointer->size())
			data_pointer = &(*container_pointer)[chunk_index];

		//准备设置反向迭代器结尾状态 is_before_start
	}

	void DynamicBitSet::BitIteratorBaseData::offset_up()
	{
		if (is_end_bit_position())
		{
			//std::cout << "BitIteratorBaseData Warning: illegal iterator plus 1 offset." << std::endl;
			return;
		}

		if (++bit_offset == std::numeric_limits<std::uint32_t>::digits)
		{
			bit_offset = 0;
			++chunk_index;
			data_pointer = &(*container_pointer)[chunk_index];
		}
	}

	void DynamicBitSet::BitIteratorBaseData::offset_down()
	{
		if (is_start_bit_position())
		{
			//std::cout << "BitIteratorBaseData Warning: illegal iterator minus 1 offset." << std::endl;
			return;
		}

		if (bit_offset == 0)
		{
			if (chunk_index > 0)
			{
				--chunk_index;
				bit_offset = std::numeric_limits<std::uint32_t>::digits - 1;
				data_pointer = &(*container_pointer)[chunk_index];
			}
		}
		else
		{
			--bit_offset;
		}
	}

	//With forward iterator
	bool DynamicBitSet::BitIteratorBaseData::is_read_only_end_iterator() const
	{
		return (is_end_iterator) && data_pointer == nullptr && (chunk_index == container_pointer->size()) && bit_offset == max_valid_bits % std::numeric_limits<std::uint32_t>::digits;
	}

	bool DynamicBitSet::BitIteratorBaseData::is_start_bit_position() const
	{
		return chunk_index == 0 && bit_offset == 0;
	}

	bool DynamicBitSet::BitIteratorBaseData::is_end_bit_position() const
	{
		return chunk_index * std::numeric_limits<std::uint32_t>::digits + bit_offset >= max_valid_bits;
	}

	void DynamicBitSet::BitIteratorBaseData::advance(std::ptrdiff_t n)
	{
		//那么我们已经到达结束位置
		if (is_end_bit_position())
		{
			chunk_index = max_valid_bits / std::numeric_limits<std::uint32_t>::digits;
			bit_offset = max_valid_bits % std::numeric_limits<std::uint32_t>::digits;
			return;
		}

		// 检查是否超出范围
		if (chunk_index * std::numeric_limits<std::uint32_t>::digits + bit_offset >= max_valid_bits)
		{
			throw std::out_of_range("Attempted to advance beyond the end of the container.");
		}
		
		// 如果n为负，转而调用advance
		if (n < 0)
		{
			retreat(-n);
			return;
		}

		// 计算总的比特位偏移
		std::size_t total_offset = static_cast<std::size_t>(n);

		// 更新块索引和位偏移
		chunk_index += total_offset / std::numeric_limits<std::uint32_t>::digits;
		bit_offset += total_offset % std::numeric_limits<std::uint32_t>::digits;

		// 如果bit_offset超过32，我们需要递增chunk_index并相应地减少bit_offset
		while (bit_offset >= std::numeric_limits<std::uint32_t>::digits)
		{
			bit_offset -= std::numeric_limits<std::uint32_t>::digits;
			++chunk_index;
		}
		
		// 检查是否超出范围，如果没有就更新数据指针
		if (container_pointer && chunk_index < container_pointer->size())
		{
			data_pointer = &(*container_pointer)[chunk_index];
		}
	}

	void DynamicBitSet::BitIteratorBaseData::retreat(std::ptrdiff_t n)
	{
		//那么我们已经到达开始位置
		if (is_start_bit_position())
			return;

		// 如果n为正，转而调用advance
		if (n < 0)
		{
			advance(-n);
			return;
		}

		// 转为正数，方便计算
		std::size_t total_offset = static_cast<std::size_t>(n);

		// 如果当前的bit_offset大于n，直接减去n
		if (bit_offset >= total_offset)
		{
			bit_offset -= total_offset;
			return;
		}

		// 否则，减去bit_offset并递减n
		total_offset -= bit_offset;
		bit_offset = 0;

		// 逐块递减
		while (total_offset >= std::numeric_limits<std::uint32_t>::digits && chunk_index > 0)
		{
			--chunk_index;
			total_offset -= std::numeric_limits<std::uint32_t>::digits;
		}

		// 如果n仍然大于0，但已经到达第一个块，那么我们已经到达开始位置
		if (total_offset > 0 && chunk_index == 0)
		{
			// 已经在开始位置，不做任何操作
			// return;
			
			throw std::out_of_range("Attempted to retreat beyond the start of the container.");
		}

		// 否则，从最后一个块的末尾开始递减
		if (total_offset > 0)
		{
			bit_offset = std::numeric_limits<std::uint32_t>::digits - total_offset;
			--chunk_index;
		}

		// 检查是否超出范围，如果没有就更新数据指针
		if (container_pointer && chunk_index < container_pointer->size())
		{
			data_pointer = &(*container_pointer)[chunk_index];
		}
	}

	/* DynamicBitSet::BitReference */

	DynamicBitSet::BitReference::BitReference() noexcept
		: data_pointer( nullptr ), bits_mask( 0 ) {}

	DynamicBitSet::BitReference::BitReference(BooleanBitWrapper* wrapper_pointer, uint32_t bits_mask) 
		: data_pointer(&(wrapper_pointer->bits)), bits_mask(bits_mask)
	{
		if(wrapper_pointer == nullptr)
			throw std::runtime_error("DynamicBitSet::BitReference::BitReference(): The pointer of the bit iterator points to the data that needs to be referenced and cannot be empty!");
	}

	DynamicBitSet::BitReference::operator bool() const noexcept
	{
		return (*data_pointer & bits_mask) != 0;
	}

	DynamicBitSet::BitReference& DynamicBitSet::BitReference::operator=(bool value) noexcept
	{
		if (value)
			*data_pointer |= bits_mask;
		else
			*data_pointer &= ~bits_mask;
		return *this;
	}

	DynamicBitSet::BitReference& DynamicBitSet::BitReference::operator=(const BitReference& other) noexcept
	{
		return *this = static_cast<bool>(other);
	}

	DynamicBitSet::BitReference& DynamicBitSet::BitReference::operator^=(const BitReference& other) noexcept
	{
		if (static_cast<bool>(*this) != static_cast<bool>(other))
			*data_pointer |= bits_mask;
		else
			*data_pointer &= ~bits_mask;
		return *this;
	}

	DynamicBitSet::BitReference& DynamicBitSet::BitReference::operator&=(const BitReference& other) noexcept
	{
		if (static_cast<bool>(*this) && static_cast<bool>(other))
			*data_pointer |= bits_mask;
		else
			*data_pointer &= ~bits_mask;
		return *this;
	}

	DynamicBitSet::BitReference& DynamicBitSet::BitReference::operator|=(const BitReference& other) noexcept
	{
		if (static_cast<bool>(*this) || static_cast<bool>(other))
			*data_pointer |= bits_mask;
		else
			*data_pointer &= ~bits_mask;
		return *this;
	}

	bool DynamicBitSet::BitReference::operator~() noexcept
	{
		return !static_cast<bool>(*this);
	}

	bool DynamicBitSet::BitReference::operator==(const BitReference& other) const
	{
		return static_cast<bool>(*this) == static_cast<bool>(other);
	}

	bool DynamicBitSet::BitReference::operator<(const BitReference& other) const
	{
		return !static_cast<bool>(*this) && static_cast<bool>(other);
	}

	bool DynamicBitSet::BitReference::operator>(const BitReference& other) const
	{
		return static_cast<bool>(*this) && !static_cast<bool>(other);
	}

	bool DynamicBitSet::BitReference::operator<=(const BitReference& other) const
	{
		return !(static_cast<bool>(*this) > static_cast<bool>(other));
	}

	bool DynamicBitSet::BitReference::operator>=(const BitReference& other) const
	{
		return !(static_cast<bool>(*this) < static_cast<bool>(other));
	}

	void DynamicBitSet::BitReference::flip() noexcept
	{
		*data_pointer ^= bits_mask;
	}

	/* DynamicBitSet::ConstantBitIterator */

	bool DynamicBitSet::ConstantBitIterator::operator*() const
	{
		if(is_read_only_end_iterator())
			throw std::out_of_range("Attempted to dereference an forward end iterator.");
		
		return (data_pointer->bits & (1 << bit_offset)) != 0;
	}

	bool DynamicBitSet::ConstantBitIterator::operator[]( std::ptrdiff_t offset ) const
	{
		if(is_read_only_end_iterator())
			throw std::out_of_range("Attempted to access an offset from an forward end iterator.");
		
		// 创建一个临时的迭代器，将其移动到指定的偏移量
		ConstantBitIterator temp = *this;
		temp.advance(offset);
		return *temp;
	}

	DynamicBitSet::ConstantBitIterator DynamicBitSet::ConstantBitIterator::operator+( std::ptrdiff_t offset ) const
	{
		ConstantBitIterator temp = *this;
		
		if(is_read_only_end_iterator())
			return temp;
		
		temp.advance(offset);
		return temp;
	}

	DynamicBitSet::ConstantBitIterator DynamicBitSet::ConstantBitIterator::operator-( std::ptrdiff_t offset ) const
	{
		ConstantBitIterator temp = *this;

		if(is_read_only_end_iterator())
			temp.is_end_iterator = false;

		if(offset == 0)
			return temp;

		if(offset > 0)
			temp.retreat(offset);
		else
			temp.advance(offset);

		return temp;
	}

	std::ptrdiff_t DynamicBitSet::ConstantBitIterator::operator-( const ConstantBitIterator& other ) const
	{
		return (chunk_index * std::numeric_limits<std::uint32_t>::digits + bit_offset) 
		- (other.chunk_index * std::numeric_limits<std::uint32_t>::digits + other.bit_offset);
	}

	DynamicBitSet::ConstantBitIterator& DynamicBitSet::ConstantBitIterator::operator++()
	{
		if ( is_read_only_end_iterator() )
			return *this;

		offset_up();
		return *this;
	}

	DynamicBitSet::ConstantBitIterator DynamicBitSet::ConstantBitIterator::operator++( int )
	{
		ConstantBitIterator temp = *this;

		if ( is_read_only_end_iterator() )
			return temp;

		offset_up();
		return temp;
	}

	DynamicBitSet::ConstantBitIterator& DynamicBitSet::ConstantBitIterator::operator--()
	{
		if(is_read_only_end_iterator())
			return *this;
    
		offset_down();
		return *this;
	}

	DynamicBitSet::ConstantBitIterator DynamicBitSet::ConstantBitIterator::operator--( int )
	{
		ConstantBitIterator temp = *this;
		
		if(is_read_only_end_iterator())
			return temp;
		
		offset_down();
		return temp;
	}

	bool DynamicBitSet::ConstantBitIterator::operator==( const ConstantBitIterator& other ) const
	{
		if (this->is_read_only_end_iterator())
			return this->chunk_index - 1 == other.chunk_index && this->bit_offset == other.bit_offset;
		if (other.is_read_only_end_iterator())
			return this->chunk_index == other.chunk_index - 1 && this->bit_offset == other.bit_offset;
		return this->data_pointer == other.data_pointer && this->bit_offset == other.bit_offset;
	}

	bool DynamicBitSet::ConstantBitIterator::operator!=( const ConstantBitIterator& other ) const
	{
		if (this->is_read_only_end_iterator())
			return this->chunk_index - 1 != other.chunk_index || this->bit_offset != other.bit_offset;
		if (other.is_read_only_end_iterator())
			return this->chunk_index != other.chunk_index - 1 || this->bit_offset != other.bit_offset;
		return this->data_pointer != other.data_pointer || this->bit_offset != other.bit_offset;
	}

	bool DynamicBitSet::ConstantBitIterator::operator<( const ConstantBitIterator& other ) const
	{
		if (this->is_read_only_end_iterator())
			return false;  // 结束迭代器不可能小于任何迭代器
		if (other.is_read_only_end_iterator())
			return true;   // 任何非结束迭代器都小于结束迭代器
		return data_pointer < other.data_pointer || (data_pointer == other.data_pointer && bit_offset < other.bit_offset);
	}

	bool DynamicBitSet::ConstantBitIterator::operator>( const ConstantBitIterator& other ) const
	{
		if (other.is_read_only_end_iterator())
			return false;  // 任何迭代器都不可能大于结束迭代器
		if (this->is_read_only_end_iterator())
			return true;   // 结束迭代器大于任何非结束迭代器
		return data_pointer > other.data_pointer || (data_pointer == other.data_pointer && bit_offset > other.bit_offset);
	}

	bool DynamicBitSet::ConstantBitIterator::operator<=( const ConstantBitIterator& other ) const
	{
		return !(*this > other);
	}

	bool DynamicBitSet::ConstantBitIterator::operator>=( const ConstantBitIterator& other ) const
	{
		return !(*this < other);
	}

	std::ptrdiff_t operator-( const DynamicBitSet::ConstantBitIterator& left, const DynamicBitSet::ConstantBitIterator& right )
	{
		return (left.chunk_index * std::numeric_limits<std::uint32_t>::digits + left.bit_offset) 
		- (right.chunk_index * std::numeric_limits<std::uint32_t>::digits + right.bit_offset);
	}

	/* DynamicBitSet::BitReference */

	DynamicBitSet::BitReference DynamicBitSet::BitIterator::operator*()
	{
		if(is_read_only_end_iterator())
			throw std::out_of_range("Attempted to dereference an forward end iterator.");
		
		return BitReference(data_pointer, uint32_t(1) << bit_offset);
	}

	DynamicBitSet::BitReference DynamicBitSet::BitIterator::operator[](std::ptrdiff_t offset)
	{
		if(is_read_only_end_iterator())
			throw std::out_of_range("Attempted to access an offset from an forward end iterator.");
		
		// 创建一个临时的迭代器，将其移动到指定的偏移量
		BitIterator temp = *this;
		temp.advance(offset);
		return *temp;
	}

	DynamicBitSet::BitIterator DynamicBitSet::BitIterator::operator+( std::ptrdiff_t offset ) const
	{
		BitIterator temp = *this;

		if ( is_read_only_end_iterator() )
			return temp;

		temp.advance( offset );
		return temp;
	}

	DynamicBitSet::BitIterator DynamicBitSet::BitIterator::operator-( std::ptrdiff_t offset ) const
	{
		BitIterator temp = *this;

		if(is_read_only_end_iterator())
			temp.is_end_iterator = false;

		if(offset == 0)
			return temp;

		if(offset > 0)
			temp.retreat(offset);
		else
			temp.advance(offset);

		return temp;
	}

	DynamicBitSet::BitIterator& DynamicBitSet::BitIterator::operator+=( std::ptrdiff_t offset )
	{
		if ( is_read_only_end_iterator() )
			return *this;

		advance( offset );
		return *this;
	}

	DynamicBitSet::BitIterator& DynamicBitSet::BitIterator::operator-=( std::ptrdiff_t offset )
	{
		if ( is_read_only_end_iterator() )
			return *this;

		retreat( offset );
		return *this;
	}

	DynamicBitSet::BitIterator& DynamicBitSet::BitIterator::operator++()
	{
		if ( is_read_only_end_iterator() )
			return *this;

		offset_up();
		return *this;
	}

	DynamicBitSet::BitIterator DynamicBitSet::BitIterator::operator++( int )
	{
		BitIterator temp = *this;

		if ( is_read_only_end_iterator() )
			return temp;

		offset_up();
		return temp;
	}

	DynamicBitSet::BitIterator& DynamicBitSet::BitIterator::operator--()
	{
		if ( is_read_only_end_iterator() )
			return *this;

		offset_down();
		return *this;
	}

	DynamicBitSet::BitIterator DynamicBitSet::BitIterator::operator--( int )
	{
		BitIterator temp = *this;

		if ( is_read_only_end_iterator() )
			return temp;

		offset_down();
		return temp;
	}

	DynamicBitSet::BitIterator& DynamicBitSet::BitIterator::operator=( bool value )
	{
		*( *this ) = value;
		return *this;
	}

	/* DynamicBitSet::ConstantReverseBitIterator */

	DynamicBitSet::ConstantReverseBitIterator::ConstantReverseBitIterator(std::vector<BooleanBitWrapper>* container_pointer, size_t max_valid_bits)
		: DynamicBitSet::BitIteratorBaseData(container_pointer, max_valid_bits, false)
	{
		is_before_start = false;  // 设置为false，因为这是反向迭代器的开始位置
	}

	DynamicBitSet::ConstantReverseBitIterator::ConstantReverseBitIterator(std::vector<BooleanBitWrapper>* container_pointer)
		: DynamicBitSet::BitIteratorBaseData(container_pointer, 0, true)
	{
		is_before_start = true;  // 设置为true，因为这是反向迭代器的结束位置
	}

	bool DynamicBitSet::ConstantReverseBitIterator::operator*() const
	{
		if(is_before_start)
			throw std::out_of_range("Attempted to dereference an backward end iterator. (forward begin - 1)");

		return (data_pointer->bits & (1 << bit_offset)) != 0;
	}

	bool DynamicBitSet::ConstantReverseBitIterator::operator[]( std::ptrdiff_t offset ) const
	{
		if(is_before_start)
			throw std::out_of_range("Attempted to access an offset from an backward end iterator. (forward begin - 1)");

		// 创建一个临时的迭代器，将其移动到指定的偏移量
		ConstantReverseBitIterator temp = *this;
		temp.retreat(offset);
		return *temp;
	}

	DynamicBitSet::ConstantReverseBitIterator DynamicBitSet::ConstantReverseBitIterator::operator+( std::ptrdiff_t offset ) const
	{
		ConstantReverseBitIterator temp = *this;
		
		if(temp.is_before_start)
		{
			if(offset >= 0)
			{
				return temp;
			}
			else
			{
				--offset;
				temp.advance(offset); //正向迭代器前向偏移
				temp.is_before_start = false;
				return temp;
			}
		}
		
		temp.retreat(offset); //正向迭代器后向偏移
		return temp;
	}

	DynamicBitSet::ConstantReverseBitIterator DynamicBitSet::ConstantReverseBitIterator::operator-( std::ptrdiff_t offset ) const
	{
		ConstantReverseBitIterator temp = *this;
		
		if(temp.is_before_start)
		{
			if(offset > 0)
			{
				--offset;
				temp.advance(offset); //正向迭代器前向偏移
				temp.is_before_start = false;
				return temp;
			}
		}
		
		return temp;
	}

	std::ptrdiff_t DynamicBitSet::ConstantReverseBitIterator::operator-( const ConstantReverseBitIterator& other ) const
	{
		return (chunk_index * std::numeric_limits<std::uint32_t>::digits + bit_offset) 
		- (other.chunk_index * std::numeric_limits<std::uint32_t>::digits + other.bit_offset);
	}

	void DynamicBitSet::ConstantReverseBitIterator::offset_up()
	{
		if ( !is_start_bit_position() && is_before_start )
		{
			is_before_start = false;
		}

		if ( is_start_bit_position() && !is_before_start )
		{
			is_before_start = true;
		}
		else
		{
			BitIteratorBaseData::offset_down();	 // 注意：反向迭代器的offset_up实际上是调用基类的offset_down
		}
	}

	void DynamicBitSet::ConstantReverseBitIterator::offset_down()
	{
		if (!is_end_bit_position())
		{
			BitIteratorBaseData::offset_up();  // 注意：反向迭代器的offset_down实际上是调用基类的offset_up
		}
	}

	DynamicBitSet::ConstantReverseBitIterator& DynamicBitSet::ConstantReverseBitIterator::operator++()
	{
		offset_up();
		return *this;
	}

	DynamicBitSet::ConstantReverseBitIterator DynamicBitSet::ConstantReverseBitIterator::operator++( int )
	{
		ConstantReverseBitIterator temp = *this;
		offset_up();
		return temp;
	}

	DynamicBitSet::ConstantReverseBitIterator& DynamicBitSet::ConstantReverseBitIterator::operator--()
	{
		offset_down();
		return *this;
	}

	DynamicBitSet::ConstantReverseBitIterator DynamicBitSet::ConstantReverseBitIterator::operator--( int )
	{
		ConstantReverseBitIterator temp = *this;
		offset_down();
		return temp;
	}

	bool DynamicBitSet::ConstantReverseBitIterator::operator==(const DynamicBitSet::ConstantReverseBitIterator& other) const
	{
		if (is_before_start || other.is_before_start)
			return is_before_start == other.is_before_start;
		
		if (this->is_read_only_end_iterator())
			return this->chunk_index - 1 == other.chunk_index && this->bit_offset == other.bit_offset;
		if (other.is_read_only_end_iterator())
			return this->chunk_index == other.chunk_index - 1 && this->bit_offset == other.bit_offset;
		return this->data_pointer == other.data_pointer && this->bit_offset == other.bit_offset;
	}

	bool DynamicBitSet::ConstantReverseBitIterator::operator!=(const DynamicBitSet::ConstantReverseBitIterator& other) const
	{
		return !(*this == other);
	}

	bool DynamicBitSet::ConstantReverseBitIterator::operator<(const DynamicBitSet::ConstantReverseBitIterator& other) const
	{
		if (is_before_start)
			return false;  // 反向 如果当前迭代器在开始之前，它不可能小于其他任何迭代器
		if (other.is_before_start)
			return true;   // 反向 任何非开始之前的迭代器都大于开始之前的迭代器
		
		// 注意，我们在这里反转了比较的方向
		if (this->is_read_only_end_iterator())
			return false;  // 正向 结束迭代器不可能小于任何迭代器
		if (other.is_read_only_end_iterator())
			return true;   // 正向 任何非结束迭代器都小于结束迭代器
		return data_pointer < other.data_pointer || (data_pointer == other.data_pointer && bit_offset < other.bit_offset);
	}

	bool DynamicBitSet::ConstantReverseBitIterator::operator>(const DynamicBitSet::ConstantReverseBitIterator& other) const
	{
		if (other.is_before_start)
			return false;  // 反向 任何迭代器都不可能大于开始之前的迭代器
		if (is_before_start)
			return true;   // 反向 开始之前的迭代器大于任何非开始之前的迭代器
		
		// 注意，我们在这里反转了比较的方向
		if (this->is_read_only_end_iterator())
			return false;  // 正向 结束迭代器不可能小于任何迭代器
		if (other.is_read_only_end_iterator())
			return true;   // 正向 任何非结束迭代器都小于结束迭代器
		return data_pointer < other.data_pointer || (data_pointer == other.data_pointer && bit_offset < other.bit_offset);
	}

	bool DynamicBitSet::ConstantReverseBitIterator::operator<=(const DynamicBitSet::ConstantReverseBitIterator& other) const
	{
		return !(*this > other);
	}

	bool DynamicBitSet::ConstantReverseBitIterator::operator>=(const DynamicBitSet::ConstantReverseBitIterator& other) const
	{
		return !(*this < other);
	}

	std::ptrdiff_t operator-( const DynamicBitSet::ConstantReverseBitIterator& left, const DynamicBitSet::ConstantReverseBitIterator& right )
	{
		return (left.chunk_index * std::numeric_limits<std::uint32_t>::digits + left.bit_offset) 
		- (right.chunk_index * std::numeric_limits<std::uint32_t>::digits + right.bit_offset);
	}

	/* DynamicBitSet::ReverseBitIterator */

	DynamicBitSet::BitReference DynamicBitSet::ReverseBitIterator::operator*()
	{
		if(is_before_start)
			throw std::out_of_range("Attempted to dereference an backward end iterator. (forward begin - 1)");

		return BitReference(data_pointer, uint32_t(1) << bit_offset);
	}

	DynamicBitSet::BitReference DynamicBitSet::ReverseBitIterator::operator[]( std::ptrdiff_t offset )
	{
		if(is_before_start)
			throw std::out_of_range("Attempted to access an offset from an backward end iterator. (forward begin - 1)");

		// 创建一个临时的迭代器，将其移动到指定的偏移量
		ReverseBitIterator temp = *this;
		temp.retreat(offset);
		return *temp;
	}

	DynamicBitSet::ReverseBitIterator DynamicBitSet::ReverseBitIterator::operator+( std::ptrdiff_t offset ) const
	{
		ReverseBitIterator temp = *this;

		if(temp.is_before_start)
		{
			if(offset >= 0)
			{
				return temp;
			}
			else
			{
				--offset;
				temp.advance(offset); //正向迭代器前向偏移
				temp.is_before_start = false;
				return temp;
			}
		}
		
		temp.retreat(offset); //正向迭代器后向偏移
		return temp;
	}

	DynamicBitSet::ReverseBitIterator DynamicBitSet::ReverseBitIterator::operator-( std::ptrdiff_t offset ) const
	{
		ReverseBitIterator temp = *this;
		
		if(temp.is_before_start)
		{
			if(offset > 0)
			{
				--offset;
				temp.advance(offset); //正向迭代器前向偏移
				temp.is_before_start = false;
			}
		}
		
		return temp;
	}

	DynamicBitSet::ReverseBitIterator& DynamicBitSet::ReverseBitIterator::operator+=(std::ptrdiff_t n)
	{
		if (n > 0)
		{
			while ( n-- && !is_before_start )
			{
				BitIteratorBaseData::offset_down();	//优化
			}
		}
		else
		{
			while ( n++ && !is_end_bit_position() )
			{
				BitIteratorBaseData::offset_up(); //优化
			}
		}
		return *this;
	}

	DynamicBitSet::ReverseBitIterator& DynamicBitSet::ReverseBitIterator::operator-=(std::ptrdiff_t n)
	{
		return *this += -n;
	}

	DynamicBitSet::ReverseBitIterator& DynamicBitSet::ReverseBitIterator::operator++()
	{
		offset_up();
		return *this;
	}

	DynamicBitSet::ReverseBitIterator DynamicBitSet::ReverseBitIterator::operator++( int )
	{
		ReverseBitIterator temp = *this;
		offset_up();
		return temp;
	}

	DynamicBitSet::ReverseBitIterator& DynamicBitSet::ReverseBitIterator::operator--()
	{
		offset_down();
		return *this;
	}

	DynamicBitSet::ReverseBitIterator DynamicBitSet::ReverseBitIterator::operator--( int )
	{
		ReverseBitIterator temp = *this;
		offset_down();
		return temp;
	}

	DynamicBitSet::ReverseBitIterator& DynamicBitSet::ReverseBitIterator::operator=(bool value)
	{
		*( *this ) = value;
		return *this;
	}

	/* DynamicBitSet Iterators */

	DynamicBitSet::iterator DynamicBitSet::begin()
	{
		return iterator(&this->bitset, 0, 0, this->valid_number_of_bits());
	}

	DynamicBitSet::iterator DynamicBitSet::end()
	{
		return iterator(&this->bitset, this->valid_number_of_bits());
	}

	DynamicBitSet::const_iterator DynamicBitSet::cbegin() const
	{
		std::cerr << "Warning: Using a const DynamicBitSet with iterator is discouraged." << std::endl;
		return const_iterator(const_cast<std::vector<BooleanBitWrapper>*>(&this->bitset), 0, 0, this->valid_number_of_bits());
	}

	DynamicBitSet::const_iterator DynamicBitSet::cend() const
	{
		std::cerr << "Warning: Using a const DynamicBitSet with iterator is discouraged." << std::endl;
		return const_iterator(const_cast<std::vector<BooleanBitWrapper>*>(&this->bitset), this->valid_number_of_bits());
	}

	DynamicBitSet::reverse_iterator DynamicBitSet::rbegin()
	{
		return reverse_iterator(&this->bitset, this->valid_number_of_bits());
	}

	DynamicBitSet::reverse_iterator DynamicBitSet::rend()
	{
		return reverse_iterator(&this->bitset);
	}

	DynamicBitSet::const_reverse_iterator DynamicBitSet::crbegin() const
	{
		std::cerr << "Warning: Using a const DynamicBitSet with iterator is discouraged." << std::endl;
		return const_reverse_iterator(const_cast<std::vector<BooleanBitWrapper>*>(&this->bitset), this->valid_number_of_bits());
	}

	DynamicBitSet::const_reverse_iterator DynamicBitSet::crend() const
	{
		std::cerr << "Warning: Using a const DynamicBitSet with iterator is discouraged." << std::endl;
		return const_reverse_iterator(const_cast<std::vector<BooleanBitWrapper>*>(&this->bitset));
	}

	// Subscript Operator for non-const DynamicBitSet
	DynamicBitSet::BitReference DynamicBitSet::operator[]( size_t index )
	{
		if ( index >= this->data_size )
			throw std::out_of_range( "Index out of range" );

		return BitReference( &(this->bitset[index / 32]), uint32_t(1) << index % 32 );
	}

	// Subscript Operator for non-const DynamicBitSet
	bool DynamicBitSet::operator[]( size_t index ) const
	{
		if ( index >= this->data_size )
			throw std::out_of_range( "Index out of range" );

		return this->bitset[ index / 32 ].bit_get( index % 32 );
	}

	inline void custom_swap( DynamicBitSet::BitReference a, DynamicBitSet::BitReference b )
	{
		bool temp = static_cast<bool>( a );
		a = b;
		b = temp;
	}

	void DynamicBitSet::custom_reverse( DynamicBitSet::BitIterator first, DynamicBitSet::BitIterator last )
	{
		try
		{
			while ( ( first != last ) && ( first != --last ) )
			{
				custom_swap( *first, *last );
				++first;
			}
		}
		catch ( const std::exception& except )
		{
			std::cout << except.what() << std::endl;
		}
	}

	//FIXME ！！！！！！
	// 按位左旋转 (<<<=)
	void DynamicBitSet::rotate_left( size_t shift )
	{
		size_t actualSize = bit_size();
		if ( shift == 0 || actualSize == 0 )
		{
			return;
		}

		shift %= actualSize;

		//???????

		this->clear_leading_bit_zeros(false);

		//data_size = this->valid_number_of_bits();
		//data_capacity = this->bitset.size() * 32;
		//data_chunk_count = this->bitset.size();
	}

	//FIXME ！！！！！！
	// 按位右旋转 (>>>=)
	void DynamicBitSet::rotate_right( size_t shift )
	{
		size_t actualSize = bit_size();
		if ( shift == 0 || actualSize == 0 )
		{
			return;
		}

		shift %= actualSize;

		size_t blockShift = shift / 32;
		size_t bitShift = shift % 32;

		//???????

		this->clear_leading_bit_zeros(false);

		//data_size = this->valid_number_of_bits();
		//data_capacity = this->bitset.size() * 32;
		//data_chunk_count = this->bitset.size();
	}

}  // namespace TwilightDream
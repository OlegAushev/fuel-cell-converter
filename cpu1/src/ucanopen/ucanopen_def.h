/**
 * @defgroup ucanopen uCANopen
 *
 * @defgroup ucanopen_app_spec Application-specific
 * @ingroup ucanopen
 *
 * @file
 * @ingroup ucanopen
 */


#pragma once


#define UCANOPEN_CAN1 mcu::CANB
#define UCANOPEN_CAN2 mcu::CANA


namespace ucanopen {
/// @addtogroup ucanopen
/// @{


/**
 * @brief uCANopen server config.
 */
struct ServerConfig
{
	uint32_t periodHeartbeat;	// uint32_t is used to be accessed via ucanopen
	uint32_t periodTpdo1;
	uint32_t periodTpdo2;
	uint32_t periodTpdo3;
	uint32_t periodTpdo4;
};


/**
 * Node ID class.
 */
class NodeId
{
public:
	const unsigned int value;
	explicit NodeId(unsigned int _value) : value(_value) {}
};


/// Network management state
enum NmtState
{
	INITIALIZING = 0x00,
	STOPPED = 0x04,
	OPERATIONAL = 0x05,
	PRE_OPERATIONAL = 0x7F
};


/// COB type
enum CobType
{
	DUMMY_COB_TYPE,
	NMT,
	SYNC,
	EMCY,
	TIME,
	TPDO1,
	RPDO1,
	TPDO2,
	RPDO2,
	TPDO3,
	RPDO3,
	TPDO4,
	RPDO4,
	TSDO,
	RSDO,
	HEARTBEAT
};


const size_t COB_TYPE_COUNT = 16;


/// COB function codes used for COB ID calculation
const uint32_t cobFunctionCode[COB_TYPE_COUNT] = {
	0x000,	// DUMMY_COB_TYPE
	0x000,	// NMT
	0x080,	// SYNC
	0x080,	// EMCY
	0x100,	// TIME
	0x180,	// TPDO1
	0x200,	// RPDO1
	0x280,	// TPDO2
	0x300,	// RPDO2
	0x380,	// TPDO3
	0x400,	// RPDO3
	0x480,	// TPDO4
	0x500,	// RPDO4
	0x580,	// TSDO
	0x600,	// RSDO
	0x700	// HEARTBEAT
};


/**
 * @brief Calculates COB ID.
 * @param cobType - COB type
 * @param nodeId - node ID
 * @return COB ID.
 */
inline uint32_t cobId(CobType cobType, unsigned int nodeId)
{
	if ((cobType == NMT) || (cobType == SYNC) || (cobType == TIME))
	{
		return cobFunctionCode[cobType];
	}
	return cobFunctionCode[cobType] + nodeId;
}


/// COB data length
const unsigned int cobDataLen[COB_TYPE_COUNT] = {
	0,	// DUMMY_COB_TYPE
	2,	// NMT
	0,	// SYNC
	2,	// EMCY
	6,	// TIME
	8,	// TPDO1
	8,	// RPDO1
	8,	// TPDO2
	8,	// RPDO2
	8,	// TPDO3
	8,	// RPDO3
	8,	// TPDO4
	8,	// RPDO4
	8,	// TSDO
	8,	// RSDO
	1	// HEARTBEAT
};


/// TPDO numbers
enum TpdoNum {TPDO_NUM1, TPDO_NUM2, TPDO_NUM3, TPDO_NUM4};


/// RPDO numbers
enum RpdoNum {RPDO_NUM1, RPDO_NUM2, RPDO_NUM3, RPDO_NUM4};


/// SDO message data union.
union CobSdoData
{
	int32_t i32;
	uint32_t u32;
	float f32;
};


/**
 * SDO message.
 */
struct CobSdo
{
	uint32_t dataSizeIndicated : 1;
	uint32_t expeditedTransfer : 1;
	uint32_t dataEmptyBytes : 2;
	uint32_t reserved : 1;
	uint32_t cs : 3;
	uint32_t index : 16;
	uint32_t subindex : 8;
	CobSdoData data;
	CobSdo()
	{
		uint64_t rawMsg = 0;
		memcpy(this, &rawMsg, sizeof(CobSdo));
	}
	CobSdo(uint64_t rawMsg) { memcpy(this, &rawMsg, sizeof(CobSdo)); }
	uint64_t all() const
	{
		uint64_t data = 0;
		memcpy(&data, this, sizeof(CobSdo));
		return data;
	}
};


/// SDO CS-codes
const uint32_t SDO_CCS_WRITE = 1;
const uint32_t SDO_SCS_WRITE = 3;
const uint32_t SDO_CCS_READ = 2;
const uint32_t SDO_SCS_READ = 2;


/// OD access possible statuses
enum ODAccessStatus
{
	OD_ACCESS_SUCCESS = 0,
	OD_ACCESS_FAIL = 1,
	OD_ACCESS_NO_ACCESS = 2
};


/// OD entry data types
enum ODEntryDataType
{
	OD_BOOL,
	OD_INT16,
	OD_INT32,
	OD_UINT16,
	OD_UINT32,
	OD_FLOAT32,
	OD_ENUM16,
	OD_TASK,
	OD_STRING
};


/// OD entry access right types
enum ODEntryAccessRight
{
	OD_ACCESS_RW,
	OD_ACCESS_RO,
	OD_ACCESS_WO,
};


/// OD entry data types sizes
const size_t ODEntryDataSizes[9] = {sizeof(bool), sizeof(int16_t), sizeof(int32_t),
		sizeof(uint16_t), sizeof(uint32_t), sizeof(float), sizeof(uint16_t), 0, 0};


/**
 * @brief OD entry key.
 */
struct ODEntryKey
{
	uint32_t index;
	uint32_t subindex;
};


/**
 * @brief OD entry value.
 */
struct ODEntryValue
{
	const char* category;
	const char* subcategory;
	const char* name;
	const char* unit;
	ODEntryDataType dataType;
	ODEntryAccessRight accessRight;
	uint32_t* dataPtr;
	ODAccessStatus (*readAccessFunc)(CobSdoData& dest);
	ODAccessStatus (*writeAccessFunc)(CobSdoData val);
};


/**
 * @brief OD entry.
 */
struct ODEntry
{
	ODEntryKey key;
	ODEntryValue value;

	/**
	 * @brief Checks OD-entry read access.
	 * @param (none)
	 * @return \c true if entry has read access, \c false otherwise.
	 */
	bool hasReadAccess() const
	{
		return (value.accessRight == OD_ACCESS_RW) || (value.accessRight == OD_ACCESS_RO);
	}


	/**
	 * @brief Checks OD-entry write access.
	 * @param (none)
	 * @return \c true if entry has write access, \c false otherwise.
	 */
	bool hasWriteAccess() const
	{
		return (value.accessRight == OD_ACCESS_RW) || (value.accessRight == OD_ACCESS_WO);
	}
};


inline bool operator<(const ODEntry& lhs, const ODEntry& rhs)
{
	return (lhs.key.index < rhs.key.index)
			|| ((lhs.key.index == rhs.key.index) && (lhs.key.subindex < rhs.key.subindex));
}


/**
 * @brief OD entry key aux class. C++03 doesn't have direct {}-initialization.
 */
struct ODEntryKeyAux
{
	uint32_t index;
	uint32_t subindex;
	ODEntryKeyAux(uint32_t _index, uint32_t _subindex) : index(_index), subindex(_subindex) {}
};


inline bool operator<(const ODEntryKeyAux& lhs, const ODEntry& rhs)
{
	return (lhs.index < rhs.key.index)
			|| ((lhs.index == rhs.key.index) && (lhs.subindex < rhs.key.subindex));
}


inline bool operator==(const ODEntryKeyAux& lhs, const ODEntry& rhs)
{
	return (lhs.index == rhs.key.index) && (lhs.subindex == rhs.key.subindex);
}


/// Used in OD-entries which doesn't have direct access to data through pointer.
#define OD_NO_DIRECT_ACCESS static_cast<uint32_t*>(NULL)


/// Used in OD-entries which have direct access to data through pointer.
#define OD_PTR(ptr) reinterpret_cast<uint32_t*>(ptr)


/// Used in OD-entries which don't have read access to data through function.
inline ODAccessStatus OD_NO_INDIRECT_READ_ACCESS(CobSdoData& dest) { return OD_ACCESS_NO_ACCESS; }


/// Used in OD-entries which don't have write access to data through function.
inline ODAccessStatus OD_NO_INDIRECT_WRITE_ACCESS(CobSdoData val) { return OD_ACCESS_NO_ACCESS; }


/// OD_TASK execution status
enum TaskStatus
{
	TASK_SUCCESS = 0,
	TASK_FAIL = 1,
	TASK_IN_PROGRESS = 2,
	TASK_STARTED = 3
};


/// @}
} // namespace ucanopen



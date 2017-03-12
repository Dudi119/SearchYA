# pragma once

#include <string>
#include <unordered_map>
#include <list>

class GeneralParams;

class ConfigParams
{
public:
	using NodeAddress = std::string;
	using NodePUMap = std::unordered_map<NodeAddress, std::list<int>>;

public:
	static ConfigParams& Instance();
	~ConfigParams(){}
	//Will fill all internal members using a received general param instance.
	//each member equivilant node name with in the general param is known and
	//hard written. the class is imutable passed this point.
	void Load(const GeneralParams& configParams);
	//Accessors
	const NodePUMap& GetProcessingUnitsMap() const { return m_processingUnitsMap; }
	int GetRecoveryTime() const { return m_recoveryTime; }
	const std::string& GetHostAddress() const { return m_hostAddress; }
	const std::string& GetMesosMasterAddress() const { return m_mesosMasterAddress; }
	std::string GetMesosMasterAddress() { return const_cast<const ConfigParams&>(*this).GetMesosMasterAddress(); }
	const std::string&  GetExecDir() const { return m_executableDir; }
	std::string GetExecDir() { return const_cast<const ConfigParams&>(*this).GetExecDir(); }
	const std::string& GetRole() const { return m_role; }

private:
	ConfigParams(){}

private:
	NodePUMap m_processingUnitsMap;
	int m_recoveryTime;
	std::string m_hostAddress;
	std::string m_mesosMasterAddress;
	int m_mesosMasterPort;
	std::string m_executableDir;
	std::string m_role;
};

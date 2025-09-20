#pragma once

#include <Iry.h>
#include "Quad.h"

class QuadGenerator : public Iry::IRGenerator<TQuad>
{
public:
	QuadGenerator(const std::string& fileName);
	void ExportDebugFormat();
protected:
	virtual void ExportInstruction(std::ofstream& exportStream, OperationID_t operation,
		TQuad& instruction) override;
	virtual const std::string OperationToStr(OperationID_t operation) override;
private:

};
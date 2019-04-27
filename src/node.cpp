#include <node.h>

#include <node_output.h>
#include <stack_allocator.h>

manta::Node::Node() {
	m_id = -1;
	m_name = "";

	m_initialized = false;
	m_evaluated = false;
}

manta::Node::~Node() {
	/* void */
}

void manta::Node::initialize() {
	if (isInitialized()) return;

	// Set initialized flag
	m_initialized = true;

	registerInputs();
	registerOutputs();

	// Initialize
	_initialize();
}

void manta::Node::evaluate() {
	if (isEvaluated()) return;

	// Set evaluated flag
	m_evaluated = true;

	// First evaluate all dependencies
	int inputCount = getInputCount();

	for (int i = 0; i < inputCount; i++) {
		const NodeOutput **node = m_inputs[i].input;
		if (node != nullptr && *node != nullptr) {
			(*node)->getParentNode()->evaluate();
		}
	}

	// Node can now self-evaluate
	_evaluate();
}

void manta::Node::destroy() {
	m_initialized = false;
	m_evaluated = false;

	_destroy();
}

void manta::Node::initializeSessionMemory(const IntersectionPoint *surfaceInteraction, NodeSessionMemory *memory, StackAllocator *stackAllocator) const {
	memset(memory, 0, sizeof(NodeSessionMemory));
}

void manta::Node::destroySessionMemory(NodeSessionMemory *memory, StackAllocator *stackAllocator) const {
	if (memory->extraMemory != nullptr) {
		stackAllocator->free(memory->extraMemory);
		memory->extraMemory = nullptr;
	}
}

void manta::Node::connectInput(const NodeOutput *nodeOutput, const char *name) {
	int inputCount = getInputCount();

	for (int i = 0; i < inputCount; i++) {
		const char *inputName = m_inputs[i].name;
		if (strcmp(inputName, name) == 0) {
			*m_inputs[i].input = nodeOutput;
			break;
		}
	}
}

manta::NodeOutput *manta::Node::getOutput(const char *name) const {
	int outputCount = getOutputCount();

	for (int i = 0; i < outputCount; i++) {
		const char *outputName = m_outputs[i].name;
		if (strcmp(outputName, name) == 0) {
			return m_outputs[i].output;
		}
	}
}

void manta::Node::_initialize() {
	/* void */
}

void manta::Node::_evaluate() {
	/* void */
}

void manta::Node::_destroy() {
	/* void */
}

void manta::Node::registerInputs() {
	/* void */
}

void manta::Node::registerOutput(NodeOutput *node, const char *name) {
	m_outputs.push_back({ node, name });
	node->setParentNode(this);
}

void manta::Node::registerOutputs() {
	/* void */
}
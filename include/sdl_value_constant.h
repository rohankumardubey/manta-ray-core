#ifndef SDL_VALUE_CONSTANT_H
#define SDL_VALUE_CONSTANT_H

#include <sdl_value.h>

#include <sdl_token_info.h>
#include <sdl_compilation_error.h>
#include <sdl_compilation_unit.h>
#include <sdl_node.h>
#include <sdl_context_tree.h>
#include <single_float_node_output.h>
#include <single_string_node_output.h>
#include <standard_allocator.h>
#include <node.h>

namespace manta {

	class SdlNode;

	template <typename T, SdlValue::VALUE_TYPE TypeCode>
	class SdlValueConstant : public SdlValue {
	protected:
		typedef T_SdlTokenInfo<T> _TokenInfo;

		static NodeOutput *generateNodeOutput(math::real_d value, SdlContextTree *context) {
			SingleFloatNodeOutput *newNode = StandardAllocator::Global()->allocate<SingleFloatNodeOutput>();
			newNode->setValue(value);

			return newNode;
		}

		static NodeOutput *generateNodeOutput(const std::string &value, SdlContextTree *context) {
			SingleStringNodeOutput *newNode = StandardAllocator::Global()->allocate<SingleStringNodeOutput>();
			newNode->setValue(value);

			return newNode;
		}

		static NodeOutput *generateNodeOutput(bool value, SdlContextTree *context) {
			// TODO
			return nullptr;
		}

		static NodeOutput *generateNodeOutput(int value, SdlContextTree *context) {
			// TODO
			return nullptr;
		}

	public:
		SdlValueConstant(const _TokenInfo &value) : SdlValue(TypeCode) { m_value = value.data; useToken(value); }
		virtual ~SdlValueConstant() { /* void */ }

		const _TokenInfo *getToken() const { return &m_token; }
		void useToken(const _TokenInfo &info) { m_value = info.data; registerToken(&info); }

		virtual void setValue(const T &value) { m_value = value; }
		T getValue() const { return m_value; }

		virtual NodeOutput *_generateNodeOutput(SdlContextTree *context) {
			return generateNodeOutput(m_value, context);
		}

	protected:
		T m_value;
		_TokenInfo m_token;
	};

	// Specialized type for labels
	class SdlValueLabel : public SdlValueConstant<std::string, SdlValue::CONSTANT_LABEL> {
	public:
		SdlValueLabel(const _TokenInfo &value) : SdlValueConstant(value) { /* void */ }
		~SdlValueLabel() { /* void */ }

		virtual SdlParserStructure *getImmediateReference(SdlContextTree *inputContext, 
				SdlCompilationError **err = nullptr, SdlContextTree **newContext = nullptr) {
			if (newContext != nullptr) *newContext = inputContext;

			SdlParserStructure *reference = nullptr;

			// First check the input context for the reference
			if (inputContext != nullptr) {
				reference = inputContext->getContext()->resolveLocalName(m_value);
				if (reference != nullptr && newContext != nullptr) 
					*newContext = inputContext->getParent();
			}

			if (reference == nullptr) {
				reference = resolveName(m_value);
			}

			// Do error checking
			if (err != nullptr) {
				if (reference == nullptr && inputContext == nullptr) {
					*err = new SdlCompilationError(m_summaryToken,
						SdlErrorCode::UnresolvedReference, inputContext);
				}
				else {
					*err = nullptr;
				}
			}

			return reference;
		}

		virtual Node *_generateNode(SdlContextTree *context) {
			SdlContextTree *newContext = context;
			SdlParserStructure *reference = getImmediateReference(context, nullptr);
			if (reference == nullptr) return nullptr;

			SdlNode *asNode = reference->getAsNode();
			if (asNode != nullptr) {
				return asNode->generateNode(context);
			}
			else return nullptr;
		}

		virtual NodeOutput *_generateNodeOutput(SdlContextTree *context) {
			SdlContextTree *newContext = context;
			SdlParserStructure *reference = getImmediateReference(context, nullptr, &newContext);
			if (reference == nullptr) return nullptr;

			SdlNode *asNode = reference->getAsNode();
			if (asNode != nullptr) {
				Node *generatedNode = generateNode(newContext);
				if (generatedNode != nullptr) {
					return generatedNode->getPrimaryOutput();
				}
			}

			// Check if this is a reference to an input (which would have to be looked up)
			SdlValue *value = nullptr;
			
			//if (!reference->isInputPoint()) {
				value = reference->getAsValue();
			//}
			//else {
			//	SdlParserStructure *inputReference = reference->getImmediateReference(context);
			//	if (inputReference != nullptr) {
			//		value = inputReference->getAsValue();
			//	}
			//}
			// TODO: if value is nullptr then that would be very bad... not sure what to do about this yet

			return value->generateNodeOutput(newContext);
		}
	};

	// Specialized type for node references
	class SdlValueNodeRef : public SdlValueConstant<SdlNode *, SdlValue::NODE_REF> {
	public:
		SdlValueNodeRef(const _TokenInfo &value) : SdlValueConstant(value) { registerComponent(value.data); }
		~SdlValueNodeRef() { /* void */ }

		virtual void setValue(SdlNode *const &value) {
			m_value = value; 
			registerComponent(value); 
		}

		virtual SdlParserStructure *getImmediateReference(SdlContextTree *inputContext, SdlCompilationError **err = nullptr, 
																						SdlContextTree **newContext = nullptr) {
			(void)inputContext;
			(void)newContext;

			if (err != nullptr) *err = nullptr;
			return m_value;
		}

		virtual SdlNode *asNode() {
			return m_value;
		}

		virtual Node *_generateNode(SdlContextTree *context) {
			return m_value->generateNode(context);
		}

		virtual NodeOutput *_generateNodeOutput(SdlContextTree *context) {
			return nullptr;
		}
	};

	typedef SdlValueConstant<int, SdlValue::CONSTANT_INT> SdlValueInt;
	typedef SdlValueConstant<std::string, SdlValue::CONSTANT_STRING> SdlValueString;
	typedef SdlValueConstant<double, SdlValue::CONSTANT_FLOAT> SdlValueFloat;
	typedef SdlValueConstant<bool, SdlValue::CONSTANT_BOOL> SdlValueBool;

} /* namespace manta */

#endif /* SDL_VALUE_CONSTANT_H */

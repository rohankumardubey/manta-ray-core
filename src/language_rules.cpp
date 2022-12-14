#include "../include/language_rules.h"

#include "../include/complex_split_node.h"
#include "../include/constructed_complex_node.h"
#include "../include/vector_split_node.h"
#include "../include/constructed_vector_node.h"
#include "../include/vector_conversions.h"
#include "../include/object_channel_types.h"
#include "../include/phong_distribution.h"
#include "../include/ggx_distribution.h"
#include "../include/disney_ggx_distribution.h"
#include "../include/disney_gtr_clearcoat_distribution.h"
#include "../include/bsdf.h"
#include "../include/simple_bsdf_material.h"
#include "../include/bilayer_brdf.h"
#include "../include/disney_diffuse_brdf.h"
#include "../include/disney_specular_brdf.h"
#include "../include/material_library.h"
#include "../include/microfacet_brdf.h"
#include "../include/microfacet_btdf.h"
#include "../include/kd_tree_node.h"
#include "../include/obj_file_node.h"
#include "../include/scene.h"
#include "../include/scene_object.h"
#include "../include/lambertian_brdf.h"
#include "../include/stratified_sampler.h"
#include "../include/random_sampler.h"
#include "../include/standard_camera_ray_emitter_group.h"
#include "../include/ray_tracer.h"
#include "../include/image_output_node.h"
#include "../include/binary_node.h"
#include "../include/unary_node.h"
#include "../include/mesh_merge_node.h"
#include "../include/sphere_primitive.h"
#include "../include/srgb_node.h"
#include "../include/image_file_node.h"
#include "../include/circular_aperture.h"
#include "../include/polygonal_aperture.h"
#include "../include/square_aperture.h"
#include "../include/fraunhofer_diffraction.h"
#include "../include/fraunhofer_diffraction_node.h"
#include "../include/convolution_node.h"
#include "../include/step_node.h"
#include "../include/padded_frame_output.h"
#include "../include/current_date_node.h"
#include "../include/date_interface_node.h"
#include "../include/lens.h"
#include "../include/simple_lens.h"
#include "../include/lens_camera_ray_emitter_group.h"
#include "../include/surface_interaction_node.h"
#include "../include/media_interface.h"
#include "../include/opaque_media_interface.h"
#include "../include/dielectric_media_interface.h"
#include "../include/microfacet_glass_bsdf.h"
#include "../include/triangle_filter.h"
#include "../include/box_filter.h"
#include "../include/gaussian_filter.h"
#include "../include/main_script_path.h"
#include "../include/script_path_node.h"
#include "../include/append_path_node.h"
#include "../include/bxdf_to_bsdf_node.h"
#include "../include/add_bxdf_node.h"
#include "../include/average_node_output.h"
#include "../include/string_conversions.h"
#include "../include/console_log_node.h"
#include "../include/image_plane.h"
#include "../include/preview_node.h"
#include "../include/session.h"
#include "../include/image_plane_converter_node.h"
#include "../include/bump_node.h"
#include "../include/perlin_noise_node.h"
#include "../include/turbulence_noise_node.h"
#include "../include/area_light.h"
#include "../include/fresnel_node.h"
#include "../include/aces_fitted_node.h"
#include "../include/hable_filmic_node.h"
#include "../include/specular_glass_bsdf.h"
#include "../include/spiral_render_pattern.h"
#include "../include/random_render_pattern.h"
#include "../include/radial_render_pattern.h"
#include "../include/progressive_resolution_render_pattern.h"
#include "../include/aperture_render_node.h"
#include "../include/complex_map_2d_node.h"
#include "../include/complex_map_interface_node.h"
#include "../include/complex_map_sampler.h"
#include "../include/complex_map_nearest_sampler_node.h"
#include "../include/complex_map_linear_sampler_node.h"
#include "../include/complex_map_sample_node.h"
#include "../include/complex_map_operation_node.h"

manta::LanguageRules::LanguageRules() {
    /* void */
}

manta::LanguageRules::~LanguageRules() {
    /* void */
}

void manta::LanguageRules::registerBuiltinNodeTypes() {
    // ====================================================
    // Builtin types
    // ====================================================

    // Channels
    registerBuiltinType<piranha::ChannelNode> (
        "__mantaray__float", &piranha::FundamentalType::FloatType);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__int", &piranha::FundamentalType::IntType);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__bool", &piranha::FundamentalType::BoolType);
    registerBuiltinType<VectorSplitNode>(
        "__mantaray__vector", &VectorNodeOutput::VectorType);
    registerBuiltinType<ComplexSplitNode>(
        "__mantaray__complex", &ComplexNodeOutput::ComplexType);
    registerBuiltinType<ComplexMapInterfaceNode>(
        "__mantaray__complex_map_2d_channel", &ObjectChannel::ComplexMap2dChannel);
    registerBuiltinType<piranha::ChannelNode> (
        "__mantaray__string", &piranha::FundamentalType::StringType);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__microfacet_distribution", &ObjectChannel::MicrofacetDistributionChannel);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__material", &ObjectChannel::MaterialChannel);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__material_library_channel", &ObjectChannel::MaterialLibraryChannel);
    registerBuiltinType<BSDF>(
        "__mantaray__bsdf", &ObjectChannel::BsdfChannel);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__bxdf", &ObjectChannel::BxdfChannel);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__mesh", &ObjectChannel::MeshChannel);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__scene_geometry", &ObjectChannel::SceneGeometryChannel);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__scene_object_channel", &ObjectChannel::SceneGeometryChannel);
    registerBuiltinType<Scene>(
        "__mantaray__scene", &ObjectChannel::SceneChannel);
    registerBuiltinType<Light>(
        "__mantaray__light", &ObjectChannel::LightChannel);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__camera", &ObjectChannel::CameraChannel);
    registerBuiltinType <piranha::ChannelNode>(
        "__mantaray__sampler", &ObjectChannel::SamplerChannel);
    registerBuiltinType<piranha::NoOpNode>(
        "__mantaray__vector_map", &VectorMap2DNodeOutput::VectorMap2dType);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__aperture", &ObjectChannel::ApertureChannel);
    registerBuiltinType<DateInterfaceNode>(
        "__mantaray__date", &DateNodeOutput::DateType);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__lens", &ObjectChannel::LensChannel);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__media_interface", &ObjectChannel::MediaInterfaceChannel);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__filter", &ObjectChannel::FilterChannel);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__image_plane", &ObjectChannel::ImagePlaneChannel);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__render_pattern", &ObjectChannel::RenderPatternChannel);
    registerBuiltinType<piranha::ChannelNode>(
        "__mantaray__complex_map_sampler", &ObjectChannel::ComplexMapSamplerChannel);

    // Constructors
    registerBuiltinType<ComplexMap2dNode>(
        "__mantaray__complex_map_2d");
    registerBuiltinType<MaterialLibrary>(
        "__mantaray__material_library");
    registerBuiltinType<RayTracer>(
        "__mantaray__ray_tracer");
    registerBuiltinType<ConstructedVectorNode>(
        "__mantaray__vector_constructor");
    registerBuiltinType<ConstructedComplexNode>(
        "__mantaray__complex_constructor");
    registerBuiltinType<KdTreeNode>(
        "__mantaray__kd_tree");
    registerBuiltinType<PhongDistribution>(
        "__mantaray__phong_distribution");
    registerBuiltinType<GgxDistribution>(
        "__mantaray__ggx_distribution");
    registerBuiltinType<DisneyGgxDistribution>(
        "__mantaray__disney_ggx_distribution");
    registerBuiltinType<DisneyGtrClearcoatDistribution>(
        "__mantaray__disney_gtr_clearcoat_distribution");
    registerBuiltinType<NullReferenceNode<BSDF>>(
        "__mantaray__null_bsdf");
    registerBuiltinType<NullReferenceNode<BXDF>>(
        "__mantaray__null_bxdf");
    registerBuiltinType<AddBxdfNode>(
        "__mantaray__add_bxdf");
    registerBuiltinType<MicrofacetBRDF>(
        "__mantaray__microfacet_brdf");
    registerBuiltinType<MicrofacetBTDF>(
        "__mantaray__microfacet_btdf");
    registerBuiltinType<SimpleBSDFMaterial>(
        "__mantaray__simple_bsdf_material");
    registerBuiltinType<BilayerBRDF>(
        "__mantaray__bilayer_brdf");
    registerBuiltinType<DisneyDiffuseBRDF>(
        "__mantaray__disney_diffuse_brdf");
    registerBuiltinType<DisneySpecularBRDF>(
        "__mantaray__disney_specular_brdf");
    registerBuiltinType<ObjFileNode>(
        "__mantaray__obj_file");
    registerBuiltinType<ObjFileNode>(
        "__mantaray__obj_file");
    registerBuiltinType<SceneObject>(
        "__mantaray__scene_object");
    registerBuiltinType<LambertianBRDF>(
        "__mantaray__lambertian_brdf");
    registerBuiltinType<StratifiedSampler>(
        "__mantaray__stratified_sampler");
    registerBuiltinType<RandomSampler>(
        "__mantaray__random_sampler");
    registerBuiltinType<StandardCameraRayEmitterGroup>(
        "__mantaray__standard_camera");
    registerBuiltinType<ImageOutputNode>(
        "__mantaray__image_output");
    registerBuiltinType<SpherePrimitive>(
        "__mantaray__sphere");
    registerBuiltinType<SrgbNode>(
        "__mantaray__srgb");
    registerBuiltinType<ACESFittedNode>(
        "__mantaray__aces_fitted");
    registerBuiltinType<HableFilmicNode>(
        "__mantaray__hable_filmic");
    registerBuiltinType<ImageFileNode>(
        "__mantaray__image_file");
    registerBuiltinType<CircularAperture>(
        "__mantaray__circular_aperture");
    registerBuiltinType<SquareAperture>(
        "__mantaray__square_aperture");
    registerBuiltinType<PolygonalAperture>(
        "__mantaray__polygonal_aperture");
    registerBuiltinType<FraunhoferDiffraction>(
        "__mantaray__fraunhofer_diffraction_legacy");
    registerBuiltinType<FraunhoferDiffractionNode>(
        "__mantaray__fraunhofer_diffraction");
    registerBuiltinType<ConvolutionNode>(
        "__mantaray__convolve_2d");
    registerBuiltinType<StepNode>(
        "__mantaray__step");
    registerBuiltinType<PaddedFrameNode>(
        "__mantaray__padded_frame");
    registerBuiltinType<CurrentDateNode>(
        "__mantaray__current_date");
    registerBuiltinType<SimpleLens>(
        "__mantaray__simple_lens");
    registerBuiltinType<LensCameraRayEmitterGroup>(
        "__mantaray__lens_camera");
    registerBuiltinType<OpaqueMediaInterface>(
        "__mantaray__opaque_media_interface");
    registerBuiltinType<DielectricMediaInterface>(
        "__mantaray__dielectric_media_interface");
    registerBuiltinType<MicrofacetGlassBSDF>(
        "__mantaray__microfacet_glass_bsdf");
    registerBuiltinType<SpecularGlassBSDF>(
        "__mantaray__specular_glass_bsdf");
    registerBuiltinType<TriangleFilter>(
        "__mantaray__triangle_filter");
    registerBuiltinType<BoxFilter>(
        "__mantaray__box_filter");
    registerBuiltinType<GaussianFilter>(
        "__mantaray__gaussian_filter");
    registerBuiltinType<MainScriptPathNode>(
        "__mantaray__main_script_path");
    registerBuiltinType<ScriptPathNode>(
        "__mantaray__script_path");
    registerBuiltinType<AppendPathNode>(
        "__mantaray__append_path");
    registerBuiltinType<PreviewNode>(
        "__mantaray__preview");
    registerBuiltinType<ImagePlane>(
        "__mantaray__standard_image_plane");
    registerBuiltinType<ImagePlaneConverterNode>(
        "__mantaray__image_plane_to_map");
    registerBuiltinType<BumpNode>(
        "__mantaray__bump");
    registerBuiltinType<PerlinNoiseNode>(
        "__mantaray__perlin_noise");
    registerBuiltinType<TurbulenceNoiseNode>(
        "__mantaray__turbulence_noise");
    registerBuiltinType<AreaLight>(
        "__mantaray__area_light");
    registerBuiltinType<SceneAddLight>(
        "__mantaray__scene_add_light");
    registerBuiltinType<FresnelNode>(
        "__mantaray__fresnel");
    registerBuiltinType<RandomRenderPattern>(
        "__mantaray__random_render_pattern");
    registerBuiltinType<SpiralRenderPattern>(
        "__mantaray__spiral_render_pattern");
    registerBuiltinType<RadialRenderPattern>(
        "__mantaray__radial_render_pattern");
    registerBuiltinType<ProgressiveResolutionRenderPattern>(
        "__mantaray__progressive_resolution_render_pattern");
    registerBuiltinType<ApertureRenderNode>(
        "__mantaray__render_aperture");
    registerBuiltinType<ComplexMapNearestSamplerNode>(
        "__mantaray__complex_map_nearest_sampler");
    registerBuiltinType<ComplexMapLinearSamplerNode>(
        "__mantaray__complex_map_linear_sampler");
    registerBuiltinType<ComplexMapOperationNode<ComplexMapOperation::Fft>>(
        "__mantaray__fft");
    registerBuiltinType<ComplexMapOperationNode<ComplexMapOperation::Roll>>(
        "__mantaray__roll");
    registerBuiltinType<ComplexMapOperationNode<ComplexMapOperation::Normalize>>(
        "__mantaray__complex_map_normalize");
    registerBuiltinType<ComplexMapOperationNode<ComplexMapOperation::DftToCft>>(
        "__mantaray__dft_to_cft");
    registerBuiltinType<ComplexMapOperationNode<ComplexMapOperation::Destroy>>(
        "__mantaray__complex_map_destroy");

    // Actions
    registerBuiltinType<piranha::ConsoleInputNode>(
        "__mantaray__console_in");
    registerBuiltinType<ConsoleOutputNode>(
        "__mantaray__console_out");
    registerBuiltinType<ComplexMapSampleNode>(
        "__mantaray__complex_map_sample");

    // Literals
    registerBuiltinType<piranha::DefaultLiteralFloatNode>(
        "__mantaray__literal_float", &piranha::FundamentalType::FloatType);
    registerBuiltinType<piranha::DefaultLiteralStringNode>(
        "__mantaray__literal_string", &piranha::FundamentalType::StringType);
    registerBuiltinType<piranha::DefaultLiteralIntNode>(
        "__mantaray__literal_int", &piranha::FundamentalType::IntType);
    registerBuiltinType<piranha::DefaultLiteralBoolNode>(
        "__mantaray__literal_bool", &piranha::FundamentalType::BoolType);

    // Conversions
    registerBuiltinType<FloatToVectorConversionNode>(
        "__mantaray__float_to_vector");
    registerBuiltinType<IntToVectorConversionNode>(
        "__mantaray__int_to_vector");
    registerBuiltinType<piranha::IntToFloatConversionNode>(
        "__mantaray__int_to_float");
    registerBuiltinType<piranha::IntToStringConversionNode>(
        "__mantaray__int_to_string");
    registerBuiltinType<piranha::StringToIntConversionNode>(
        "__mantaray__string_to_int");

    registerBuiltinType<BxdfToBsdfNode>(
        "__mantaray__bxdf_to_bsdf");

    registerBuiltinType<VectorToStringConversionNode>(
        "__mantaray__vector_to_string");

    // Unary operations
    registerBuiltinType<piranha::NumNegateOperationNode<piranha::native_float>>(
        "__mantaray__float_negate");
    registerBuiltinType<VectorNegateNode>(
        "__mantaray__vector_negate");
    registerBuiltinType<VectorMagnitudeNode>(
        "__mantaray__vector_magnitude");
    registerBuiltinType<VectorNormalizeNode>(
        "__mantaray__vector_normalize");
    registerBuiltinType<VectorMaxComponentNode>(
        "__mantaray__vector_max_component");
    registerBuiltinType<VectorSinNode>(
        "__mantaray__vector_absolute");
    registerBuiltinType<VectorSinNode>(
        "__mantaray__vector_sin");

    registerBuiltinType<UvWrapNode>(
        "__mantaray__uv_wrap");

    registerBuiltinType<AverageNode>(
        "__mantaray__vector_average");

    // Binary operations
    registerBuiltinType<AddNode>(
        "__mantaray__vector_add");
    registerBuiltinType<SubtractNode>(
        "__mantaray__vector_sub");
    registerBuiltinType<DivideNode>(
        "__mantaray__vector_div");
    registerBuiltinType<MultiplyNode>(
        "__mantaray__vector_mul");
    registerBuiltinType<DotNode>(
        "__mantaray__vector_dot");
    registerBuiltinType<CrossNode>(
        "__mantaray__vector_cross");
    registerBuiltinType<PowerNode>(
        "__mantaray__vector_pow");
    registerBuiltinType<MaxNode>(
        "__mantaray__vector_max");
    registerBuiltinType<MinNode>(
        "__mantaray__vector_min");
    registerBuiltinType<MeshMergeNode>(
        "__mantaray__mesh_merge");

    // -- Float operations
    registerBuiltinType<piranha::OperationNodeSpecialized<
        piranha::native_float, piranha::DivideOperationNodeOutput>>("__mantaray__float_divide");

    // -- Int operations
    registerBuiltinType<piranha::OperationNodeSpecialized<
        piranha::native_int, piranha::MultiplyOperationNodeOutput>>("__mantaray__int_multiply");
    registerBuiltinType<piranha::OperationNodeSpecialized<
        piranha::native_int, piranha::AddOperationNodeOutput>>("__mantaray__int_add");
    registerBuiltinType<piranha::OperationNodeSpecialized<
        piranha::native_int, piranha::SubtractOperationNodeOutput>>("__mantaray__int_sub");
    registerBuiltinType<piranha::NumNegateOperationNode<
        piranha::native_int>>("__mantaray__int_negate");

    // -- String operations
    registerBuiltinType<piranha::OperationNodeSpecialized<
        piranha::native_string, piranha::AddOperationNodeOutput>>("__mantaray__string_add");

    // Surface interaction
    registerBuiltinType<SurfaceInteractionNode>(
        "__mantaray__surface_interaction");

    // ====================================================
    // Literal types
    // ====================================================
    registerLiteralType(piranha::LiteralType::Float, "__mantaray__literal_float");
    registerLiteralType(piranha::LiteralType::String, "__mantaray__literal_string");
    registerLiteralType(piranha::LiteralType::Integer, "__mantaray__literal_int");
    registerLiteralType(piranha::LiteralType::Boolean, "__mantaray__literal_bool");

    // ====================================================
    // Conversions
    // ====================================================
    registerConversion(
        { &piranha::FundamentalType::FloatType, &VectorNodeOutput::VectorType },
        "__mantaray__float_to_vector"
    );
    registerConversion(
        { &piranha::FundamentalType::IntType, &VectorNodeOutput::VectorType },
        "__mantaray__int_to_vector"
    );
    registerConversion(
        { &piranha::FundamentalType::IntType, &piranha::FundamentalType::FloatType },
        "__mantaray__int_to_float"
    );
    registerConversion(
        { &piranha::FundamentalType::IntType, &piranha::FundamentalType::StringType },
        "__mantaray__int_to_string"
    );
    registerConversion(
        { &piranha::FundamentalType::StringType, &piranha::FundamentalType::IntType },
        "__mantaray__string_to_int"
    );
    registerConversion(
        { &VectorNodeOutput::VectorType, &piranha::FundamentalType::StringType },
        "__mantaray__vector_to_string"
    );

    registerConversion(
        { &ObjectChannel::BxdfChannel, &ObjectChannel::BsdfChannel },
        "__mantaray__bxdf_to_bsdf"
    );

    // ====================================================
    // Binary operators
    // ====================================================

    // Vector operators
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Mul, &VectorNodeOutput::VectorType, &VectorNodeOutput::VectorType },
        "__mantaray__vector_mul"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Mul, &VectorNodeOutput::VectorType, &piranha::FundamentalType::FloatType },
        "__mantaray__vector_mul"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Mul, &VectorNodeOutput::VectorType, &piranha::FundamentalType::IntType },
        "__mantaray__vector_mul"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Div, &VectorNodeOutput::VectorType, &VectorNodeOutput::VectorType },
        "__mantaray__vector_div"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Div, &VectorNodeOutput::VectorType, &piranha::FundamentalType::FloatType },
        "__mantaray__vector_div"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Div, &VectorNodeOutput::VectorType, &piranha::FundamentalType::IntType },
        "__mantaray__vector_div"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Add, &VectorNodeOutput::VectorType, &VectorNodeOutput::VectorType },
        "__mantaray__vector_add"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Add, &VectorNodeOutput::VectorType, &piranha::FundamentalType::FloatType },
        "__mantaray__vector_add"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Add, &VectorNodeOutput::VectorType, &piranha::FundamentalType::IntType },
        "__mantaray__vector_add"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Sub, &VectorNodeOutput::VectorType, &VectorNodeOutput::VectorType },
        "__mantaray__vector_sub"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Sub, &VectorNodeOutput::VectorType, &piranha::FundamentalType::FloatType },
        "__mantaray__vector_sub"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Sub, &VectorNodeOutput::VectorType, &piranha::FundamentalType::IntType },
        "__mantaray__vector_sub"
    );

    // Float operators
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Div, &piranha::FundamentalType::FloatType, &piranha::FundamentalType::FloatType },
        "__mantaray__float_divide"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Div, &piranha::FundamentalType::FloatType, &piranha::FundamentalType::IntType },
        "__mantaray__float_divide"
    );

    // Int operators
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Mul, &piranha::FundamentalType::IntType, &piranha::FundamentalType::IntType },
        "__mantaray__int_multiply"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Add, &piranha::FundamentalType::IntType, &piranha::FundamentalType::IntType },
        "__mantaray__int_add"
    );
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Sub, &piranha::FundamentalType::IntType, &piranha::FundamentalType::IntType },
        "__mantaray__int_sub"
    );

    // String operators
    registerOperator(
        { piranha::IrBinaryOperator::Operator::Add, &piranha::FundamentalType::StringType, &piranha::FundamentalType::StringType },
        "__mantaray__string_add"
    );

    // ====================================================
    // Unary operators
    // ====================================================
    registerUnaryOperator(
        { piranha::IrUnaryOperator::Operator::NumericNegate, &piranha::FundamentalType::IntType },
        "__mantaray__int_negate"
    );
    registerUnaryOperator(
        { piranha::IrUnaryOperator::Operator::NumericNegate, &piranha::FundamentalType::FloatType },
        "__mantaray__float_negate"
    );
    registerUnaryOperator(
        { piranha::IrUnaryOperator::Operator::NumericNegate, &VectorNodeOutput::VectorType },
        "__mantaray__vector_negate"
    );
}

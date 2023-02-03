#include "Stable.h"

QString GetTypeName( int type )
{
    switch ( type )
    {

        case 1:
            return "UF_dummy_type";
            break;

        case 2:
            return "UF_point_type";
            break;




        case 3:
            return "UF_line_type";
            break;





        case 4:
            return "UF_solid_collection_type";
            break;

        case 5:
            return "UF_circle_type";
            break;




        case 6:
            return "UF_conic_type";
            break;




        case 7:
            return "UF_spcurve_type";
            break;




        case 8:
            return "UF_old_spline_type";
            break;



        case 9:
            return "UF_spline_type";
            break;


        case 10:
            return "UF_pattern_type";
            break;



        case 11:
            return "UF_part_attribute_type";
            break;



        case 12:
            return "UF_layer_category_type";
            break;

        case 13:
            return "UF_kanji_type";
            break;

        case 14:
            return "UF_boundary_type";
            break;

        case 15:
            return "UF_group_type";
            break;

        case 16:
            return "UF_cylinder_type";
            break;



        case 17:
            return "UF_cone_type";
            break;



        case 18:
            return "UF_sphere_type";
            break;



        case 19:
            return "UF_surface_of_revolution_type";
            break;





        case 20:
            return "UF_tabulated_cylinder_type";
            break;



        case 21:
            return "UF_ruled_surface_type";
            break;



        case 22:
            return "UF_bounded_plane_type";
            break;



        case 23:
            return "UF_blended_face_type";
            break;



        case 24:
            return "UF_sculptured_surface_type";
            break;



        case 25:
            return "UF_drafting_entity_type";
            break;

























        case 26:
            return "UF_dimension_type";
            break;





















        case 27:
            return "UF_font_table_type";
            break;

        case 28:
            return "UF_color_table_type";
            break;

        case 29:
            return "UF_margin_type";
            break;

        case 31:
            return "UF_gfem_load_type";
            break;








        case 32:
            return "UF_gfem_node_type";
            break;





        case 33:
            return "UF_gfem_element_type";
            break;












        case 34:
            return "UF_gfem_ferd_type";
            break;


        case 35:
            return "UF_gfem_annotation_type";
            break;





        case 36:
            return "UF_gfem_control_node_list_type";
            break;


        case 37:
            return "UF_gfem_material_type";
            break;


        case 38:
            return "UF_instance_type";
            break;

        case 39:
            return "UF_connection_type";
            break;




        case 40:
            return "UF_prefix_type";
            break;


        case 41:
            return "UF_tool_type";
            break;

        case 42:
            return "UF_sub_entity_for_dim_type";
            break;




        case 43:
            return "UF_b_surface_type";
            break;



        case 44:
            return "UF_validation_type";
            break;

        case 45:
            return "UF_coordinate_system_type";
            break;





        case 46:
            return "UF_plane_type";
            break;

        case 47:
            return "UF_bounded_plane_loop_type";
            break;


        case 48:
            return "UF_report_entity_type";
            break;


        case 49:
            return "UF_report_entry_entity_type";
            break;


        case 50:
            return "UF_nesting_type";
            break;

        case 51:
            return "UF_tool_display_type";
            break;



        case 52:
            return "UF_skeleton_ent_type";
            break;










        case 53:
            return "UF_parameter_type";
            break;












































































        case 54:
            return "UF_device_type";
            break;



        case 55:
            return "UF_matrix_type";
            break;

        case 56:
            return "UF_gear_type";
            break;

        case 57:
            return "UF_gear_mesh_type";
            break;

        case 58:
            return "UF_gear_train_type";
            break;

        case 59:
            return "UF_fatigue_type";
            break;


        case 60:
            return "UF_view_type";
            break;



















        case 61:
            return "UF_layout_type";
            break;



        case 62:
            return "UF_drawing_type";
            break;

        case 63:
            return "UF_component_type";
            break;




        case 64:
            return "UF_reference_set_type";
            break;









        case 65:
            return "UF_offset_surface_type";
            break;



        case 66:
            return "UF_foreign_surface_type";
            break;



        case 67:
            return "UF_foreign_surface_data_type";
            break;

        case 68:
            return "UF_occ_instance_type";
            break;


        case 69:
            return "UF_occ_shadow_part_type";
            break;


        case 70:
            return "UF_solid_type";
            break;







        case 71:
            return "UF_face_type";
            break;













        case 72:
            return "UF_edge_type";
            break;



        case 73:
            return "UF_solid_composite_type";
            break;

        case 74:
            return "UF_history_type";
            break;

        case 75:
            return "UF_gfem_post_data_type";
            break;


        case 76:
            return "UF_gfem_post_ferd_type";
            break;




        case 77:
            return "UF_gfem_post_display_type";
            break;




        case 78:
            return "UF_gfem_post_group_type";
            break;


        case 79:
            return "UF_gfem_outline_type";
            break;


        case 80:
            return "UF_gfem_local_csys_type";
            break;




        case 81:
            return "UF_gfem_loader_type";
            break;


        case 82:
            return "UF_sketch_tol_equation_type";
            break;










        case 83:
            return "UF_sketch_tol_variable_type";
            break;

















        case 84:
            return "UF_sketch_tol_output_reqst_type";
            break;

        case 85:
            return "UF_mdm_mechanism_type";
            break;

        case 86:
            return "UF_mdm_joint_type";
            break;





































        case 87:
            return "UF_mdm_link_type";
            break;

        case 88:
            return "UF_mdm_spring_type";
            break;









        case 89:
            return "UF_mdm_motion_vector_type";
            break;

        case 90:
            return "UF_mdm_force_type";
            break;

        case 91:
            return "UF_shaft_stress_type";
            break;
























        case 92:
            return "UF_shaft_feature_type";
            break;

        case 93:
            return "UF_shaft_load_type";
            break;

        case 94:
            return "UF_shaft_support_type";
            break;

        case 95:
            return "UF_shaft_section_type";
            break;

        case 96:
            return "UF_shaft_type";
            break;

        case 97:
            return "UF_mdm_analysis_pt_type";
            break;






        case 98:
            return "UF_mdm_damper_type";
            break;




        case 99:
            return "UF_mdm_torque_type";
            break;


        case 30:
            return "UF_gfem_property_set_type";
            break;





        case 100:
            return "UF_machining_operation_type";
            break;









































        case 101:
            return "UF_machining_path_type";
            break;


        case 102:
            return "UF_table_column_type";
            break;

        case 103:
            return "UF_machining_ude_map_type";
            break;

        case 104:
            return "UF_data_declaration_type";
            break;

        case 105:
            return "UF_machining_geometry_grp_type";
            break;

























        case 106:
            return "UF_machining_mach_tool_grp_type";
            break;



























        case 107:
            return "UF_machining_parameter_set_type";
            break;































        case 108:
            return "UF_last_operation_pointer_type";
            break;

        case 109:
            return "UF_machining_tool_type";
            break;




        case 110:
            return "UF_machining_global_data_type";
            break;

        case 111:
            return "UF_machining_geom_type";
            break;














        case 112:
            return "UF_machining_null_grp_type";
            break;

        case 113:
            return "UF_smart_container_type";
            break;






        case 114:
            return "UF_machining_boundary_member_type";
            break;





        case 115:
            return "UF_machining_master_operation_type";
            break;











        case 116:
            return "UF_machining_post_command_type";
            break;











        case 117:
            return "UF_machining_suboperation";
            break;













        case 118:
            return "UF_machining_boundary_type";
            break;



        case 119:
            return "UF_machining_control_event_type";
            break;













































































































        case 120:
            return "UF_machining_ncm_type";
            break;






        case 121:
            return "UF_machining_task_type";
            break;




        case 122:
            return "UF_machining_setup_type";
            break;



        case 123:
            return "UF_machining_feedrate_type";
            break;

        case 124:
            return "UF_machining_display_type";
            break;

        case 125:
            return "UF_machining_dp_type";
            break;































        case 126:
            return "UF_machining_pathindex_type";
            break;

        case 127:
            return "UF_machining_tldsp_type";
            break;

        case 128:
            return "UF_machining_mode_type";
            break;















        case 129:
            return "UF_machining_clip_type";
            break;

        case 130:
            return "UF_render_set_type";
            break;

        case 131:
            return "UF_sketch_tol_csys_type";
            break;




        case 132:
            return "UF_sketch_tol_feature_type";
            break;

        case 133:
            return "UF_sketch_tol_mating_type";
            break;

        case 134:
            return "UF_sketch_type";
            break;




        case 135:
            return "UF_ordinate_margin_type";
            break;



        case 136:
            return "UF_phys_material_type";
            break;

        case 137:
            return "UF_ug_libraries_type";
            break;

        case 138:
            return "UF_faceted_model_data_type";
            break;

        case 139:
            return "UF_faceted_model_type";
            break;








        case 140:
            return "UF_flange_type";
            break;



        case 141:
            return "UF_bend_type";
            break;



        case 142:
            return "UF_flat_pattern_type";
            break;



        case 143:
            return "UF_sheet_metal_type";
            break;

        case 144:
            return "UF_table_type";
            break;



        case 145:
            return "UF_mdm_genforce_type";
            break;





        case 146:
            return "UF_sfem_composite_type";
            break;







        case 147:
            return "UF_cam_cut_method_type";
            break;

        case 148:
            return "UF_dimension_set_type";
            break;



        case 149:
            return "UF_display_object_type";
            break;

        case 150:
            return "UF_mdm_curve_curve_contact_type";
            break;

        case 151:
            return "UF_prefix1_type";
            break;




        case 152:
            return "UF_symbol_master_type";
            break;


        case 153:
            return "UF_logic_part_master_type";
            break;

        case 0:
            return "UF_drafting_search_criteria_type";
            break;

        case 154:
            return "UF_draft_callout_type";
            break;

        case 155:
            return "UF_smsp_product_definition_type";
            break;










        case 156:
            return "UF_symbol_type";
            break;


        case 157:
            return "UF_logic_part_type";
            break;

        case 158:
            return "UF_smart_model_instance_type";
            break;






        case 159:
            return "UF_datum_reference_frame_type";
            break;



        case 160:
            return "UF_constraint_type";
            break;




















        case 161:
            return "UF_connection1_type";
            break;












        case 162:
            return "UF_node_type";
            break;

        case 163:
            return "UF_report_net_list_type";
            break;

        case 164:
            return "UF_traceline_type";
            break;



        case 165:
            return "UF_tabular_note_type";
            break;




























        case 166:
            return "UF_cam_material_type";
            break;

        case 167:
            return "UF_rlist_type";
            break;





        case 168:
            return "UF_route_route_type";
            break;












        case 169:
            return "UF_analysis_type";
            break;













        case 170:
            return "UF_report_type";
            break;


        case 171:
            return "UF_cam_type";
            break;


        case 172:
            return "UF_cam_body_type";
            break;


        case 173:
            return "UF_cam_follower_type";
            break;


        case 174:
            return "UF_machining_mfgftr_type";
            break;

        case 175:
            return "UF_cam_solid_cross_section";
            break;

        case 176:
            return "UF_mdm_contact_type";
            break;


        case 177:
            return "UF_drafting_image_type";
            break;

        case 178:
            return "UF_tracking_point_type";
            break;





        case 179:
            return "UF_tool_holder_type";
            break;

        case 180:
            return "UF_material_type";
            break;


        case 181:
            return "UF_texture_type";
            break;

        case 182:
            return "UF_light_source_type";
            break;

        case 183:
            return "UF_curve_group_type";
            break;






        case 184:
            return "UF_general_face_set_type";
            break;

        case 185:
            return "UF_anim_traj_type";
            break;

        case 186:
            return "UF_sheet_group_type";
            break;

        case 187:
            return "UF_cs2_rigid_set_type";
            break;

        case 188:
            return "UF_design_rule_type";
            break;






        case 189:
            return "UF_thd_symbolic_data_type";
            break;

        case 190:
            return "UF_foreign_surf_type";
            break;

        case 191:
            return "UF_user_defined_object_type";
            break;

        case 192:
            return "UF_generic_ent_int_sub_ent_type";
            break;







        case 193:
            return "UF_generic_ent_real_sub_ent_type";
            break;

        case 194:
            return "UF_symbol_font_type";
            break;

        case 195:
            return "UF_dataum_point_type";
            break;

        case 196:
            return "UF_datum_axis_type";
            break;

        case 197:
            return "UF_datum_plane_type";
            break;

        case 198:
            return "UF_solid_section_type";
            break;

        case 199:
            return "UF_section_edge_type";
            break;



        case 200:
            return "UF_section_segment_type";
            break;




        case 201:
            return "UF_solid_silhouette_type";
            break;




        case 202:
            return "UF_section_line_type";
            break;








        case 203:
            return "UF_solid_in_view_type";
            break;

        case 204:
            return "UF_component_set_type";
            break;

        case 205:
            return "UF_feature_type";
            break;


        case 206:
            return "UF_zone_type";
            break;



        case 207:
            return "UF_filter_type";
            break;



        case 208:
            return "UF_promotion_type";
            break;


        case 209:
            return "UF_mdm_measure_type";
            break;

        case 210:
            return "UF_mdm_trace_type";
            break;

        case 211:
            return "UF_mdm_interference_type";
            break;

        case 212:
            return "UF_script_type";
            break;

        case 213:
            return "UF_spreadsheet_type";
            break;



        case 214:
            return "UF_reference_type";
            break;

        case 215:
            return "UF_scalar_type";
            break;

        case 216:
            return "UF_offset_type";
            break;

        case 217:
            return "UF_direction_type";
            break;

        case 218:
            return "UF_parametric_text_type";
            break;




        case 219:
            return "UF_xform_type";
            break;

        case 220:
            return "UF_route_control_point_type";
            break;

        case 221:
            return "UF_route_port_type";
            break;

        case 222:
            return "UF_route_segment_type";
            break;






        case 223:
            return "UF_route_connection_type";
            break;

        case 224:
            return "UF_route_stock_type";
            break;





        case 225:
            return "UF_route_part_anchor_type";
            break;

        case 226:
            return "UF_route_cross_section_type";
            break;

        case 227:
            return "UF_route_stock_data_type";
            break;

        case 228:
            return "UF_route_corner_type";
            break;






        case 229:
            return "UF_route_part_type_type";
            break;

        case 230:
            return "UF_fam_type";
            break;



        case 231:
            return "UF_fam_attr_type";
            break;













        case 232:
            return "UF_sfem_mesh_type";
            break;




        case 233:
            return "UF_sfem_mesh_recipe_type";
            break;






        case 234:
            return "UF_faceset_type";
            break;

        case 235:
            return "UF_sfem_mesh_geometry_type";
            break;









        case 236:
            return "UF_feature_cache_type";
            break;

        case 237:
            return "UF_sfem_load_type";
            break;




        case 238:
            return "UF_sfem_bndcond_type";
            break;




        case 239:
            return "UF_sfem_property_type";
            break;





        case 240:
            return "UF_sfem_property_name_type";
            break;

        case 241:
            return "UF_axis_type";
            break;

        case 242:
            return "UF_cs2_vertex_type";
            break;

        case 243:
            return "UF_cs2_constraint_type";
            break;















        case 244:
            return "UF_cs2_constraint_system_type";
            break;



        case 245:
            return "UF_attribute_category_type";
            break;

        case 246:
            return "UF_attribute_type";
            break;










        case 247:
            return "UF_note_type";
            break;




        case 248:
            return "UF_tol_feature_instance_type";
            break;













        case 249:
            return "UF_engineering_text_type";
            break;

        case 250:
            return "UF_annotation_type";
            break;





        case 251:
            return "UF_tolerance_feature_type";
            break;






        case 252:
            return "UF_leader_type";
            break;

        case 253:
            return "UF_engineering_symbol_type";
            break;


        case 254:
            return "UF_feature_control_frame_type";
            break;

























        case 255:
            return "UF_max_entity_type";
            break;

        case UF_OBJ_EXPR_TYPE:
            return "UF_OBJ_EXPR_TYPE";
            break;

        default:
            return "UNKNOWN";
    }




    return QString();
}

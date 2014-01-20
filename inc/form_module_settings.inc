<?php
/** 
 * menurolerights module include file  
 * 
 * Form 'module settings'
 *
 * @author Wolfgang Hauertmann <info@hauertmann.com>
 * @copyright (c) 2014 Wolfgang Hauertmann
 * @license http://www.gnu.org/licenses/gpl-2.0.html GPL-2.0
 * 
 */

/**
 * Implements hook_form();
 * 
 * Formular for module settings
 * 
 * @param array $form 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing.
 * @return void 
 */
function menurolerights_form($form, &$form_state) {
  // Get a list of menu titles indexed by internal menu-names
  $menu_list = _hrmrr_getListOfMenuTitles();  
  // Get last selected menu-name  
  $menu_name = _hrmrr_getLastSelectedMenuName($form_state); 
  // Get menu links of selected menu-name
  $menu_links = _hrmrr_getMenuLinksOfSelectedMenuName($menu_name);
  // On first call set page to 1, on second call to 2
  $page = _hrmrr_setPageState($menu_name, $menu_links, $form_state);
  // Define submit handler
  $form['#submit'] = array('_menurolerights_form_submit');
  // Build form elements depending on page state (first=1, second call=2)
  _hrmrr_buildFormElementsDependingOnPageState($page, $menu_name, $menu_list, $menu_links,$form, $form_state);
  // Build table of defined rules 
  _hrmrr_buildTableOfDefinedRules($menu_name, $menu_list, $page, $form, $form_state);
  // Standard renderer für admin-settings-menu 
  // (if you want to have the submit button on the right side, which is not sensfull here) 
  // $form = system_settings_form($form); 
  return $form;
}

/**
 * Implements hook_form_submit();
 * 
 * Handle the submit of formular for module settings
 * 
 * @param array $form 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing.
 * @return void 
 */
function _menurolerights_form_submit($form, &$form_state) {
  $action = isset($form_state['values']['sel_tab_action'])?$form_state['values']['sel_tab_action']:SEL_NONE;
  switch($action) {
      
    case SEL_NONE:
      _hrmrr_menurolerights_form_submit_SEL_NONE($form, $form_state);
      break;
    
    case SEL_DELETE:
      _hrmrr_menurolerights_form_submit_SEL_DELETE($form, $form_state);
      break;
  
    case SEL_ACTIVE:
      _hrmrr_menurolerights_form_submit_SEL_ACTIVE($form, $form_state);
      break;
  
    case SEL_INACTIVE:
      _hrmrr_menurolerights_form_submit_SEL_INACTIVE($form, $form_state);
      break;
  
    case SEL_CHANGE:
      _hrmrr_menurolerights_form_submit_SEL_CHANGE($form, $form_state);
      break;
  
  }
}
         
/**
* Implements hook_form_validate
* 
* Validation from the Form API.
*
 * @param array $form 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing.
* @return void 
*/
function menurolerights_form_validate($form, &$form_state){ 
  if (isset($form_state['values']['menurolerights_rule_menu_name'])) {
    $menu_id = $form_state['values']['menurolerights_rule_menu_name'];
    if (!is_string($menu_id)){
      form_set_error('mlid_id', t('You must enter a string for the menu name.'));
    }
  }
}





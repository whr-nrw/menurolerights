<?php
/** 
 * menurolerights module include file  
 * 
 * Alter the menu overview form
 *
 * @author Wolfgang Hauertmann <info@hauertmann.com>
 * @copyright (c) 2014 Wolfgang Hauertmann
 * @license http://www.gnu.org/licenses/gpl-2.0.html GPL-2.0
 * 
 */

/**
 * Alter the menu overview form
 *
 * User actions are accepted or rejected depending on their role rights
 * 
 * @param array $form (call by ref) 
 *   Description array of all form elements
 * @return void 
 */
function _menurolerights_form_alter_menu_overview_form(&$form) {
  global $user;  
  // Get Rules
  $rules = _hrmrr_getListOfActiveRules();
  // Over all active rules ...
  foreach ($rules as $rule) {
    // if rule matches one role of the user ...
    if (in_array($rule['role'], $user->roles)) {
      // apply validation function and menu informations to form
      _menurolerights_prepairFormForValidation($form, $rule);
      // reject or accept user actions depending on allowed access areas of menu
      _menurolerights_prepairFormForUserAccess($form, $rule);
    }
  }
}

/**
 * Helper function for _menurolerights_form_alter_menu_overview_form
 * @todo better using $form_state['storage'] for storing special parameters?
 * 
 * @param array $form (call by ref) 
 *   Description array of all form elements
 * @param array $rule 
 *   Contains one allowed menu area for a given menu
 * @return void 
 */
function _menurolerights_prepairFormForValidation(&$form, $rule) {
  // Declare an additional validation procedure  
  $form['#validate'][] = '_menurolerights_menu_overview_form_submit_validate';
  // Store allowed menu areas for validation  
  $form['master_mlid'] =  array(
    '#type' => 'hidden',
    '#value' => $rule['master_mlid'],
  );
  $form['menu_name'] =  array(
    '#type' => 'hidden',
    '#value' => $rule['menu_name'],
  );
}

/**
 * Helper function for _menurolerights_form_alter_menu_overview_form
 * 
 * @param array $form (call by ref) 
 *   Description array of all form elements
 * @param array $rule 
 *   Contains one allowed menu area for a given menu
 * @return void 
 */
function _menurolerights_prepairFormForUserAccess(&$form, $rule) {
  // Over all form elements ...
  foreach ($form as $data) { 
    if ( // element is a menu item from menu ,,,	
         (isset($data['#item']) 
       && isset($data['#item']['menu_name']) 
       && ($data['#item']['menu_name'] === $rule['menu_name'])) 
         // ... and menu item is not child of the parent item ...
       && (!_menurolerights_menuIsChildOfMasterMenuItem($data['#item'], $rule['master_mlid'])) 
    ) {
      // ... then disable user access
      _menurolerights_disableUserAccessToMenuItem($form, $data['#item']['mlid']);
    }
  }
}

/**
 * Helper function for _menurolerights_prepairFormForUserAccess
 * 
 * 
 * @param array $item 
 *   Menu item to test on 'is submenu in allowed menu area'
 * @param int $master_mlid 
 *   Menu item id under which a role has access on submenus
 * @return void 
 */
function _menurolerights_menuIsChildOfMasterMenuItem($item, $master_mlid) {
  $isChild = false;
  // Over all parent items ...
  for ($i = 1; $i <= 10000; $i++) {
    // if a list of the parent path not exists, e.g. p1=123, p2=234, ..., break
    if (!isset($item['p' . $i])) {
      break;
    } // at least 'p1' exists, if a hierachical menu exists
    // Test on 'master-mlid is parent of actuel menu item' ...
    if (($item['mlid'] != $item['p' . $i]) && ($master_mlid == $item['p' . $i])) {
      $isChild = true;
      break;
    }
  }
  return $isChild;
}

/**
 * Helper function for _menurolerights_prepairFormForUserAccess
 * 
 * @param array $form (call by ref) 
 *   Description array of all form elements
 * @param int $mlid 
 *   Menu item id of menu item which should be disabled
 * @return void 
 */
function _menurolerights_disableUserAccessToMenuItem(&$form, $mlid) {
  // Don't show the operations: edit, delete
  $form['mlid:' . $mlid]['operations'] = array();
  // Don't show: weight
  $form['mlid:' . $mlid]['weight'] = array(
    '#type' => 'weight',
    '#delta' => $form['mlid:' . $mlid]['weight']['#delta'],
    '#default_value' => $form['mlid:' . $mlid]['weight']['#default_value'],
    '#title_display' => $form['mlid:' . $mlid]['weight']['#title_display'],
    '#title' => $form['mlid:' . $mlid]['weight']['#title'],
    '#disabled' => true,
    // '#access' => false,  // Sichtbarkeit von weight erforderlich wegen Tabledrag-Ajax (sonst funktioniert verschieben nicht richtig)
  );
  // Disable checkboxes 'menu item is active' 
  $form['mlid:' . $mlid]['hidden'] = array(
    '#type' => 'checkbox',
    '#disabled' => true,
    '#default_value' => $form['mlid:' . $mlid]['hidden']['#default_value'],
  );
}

/**
 * Additional handler for validation of the menu overview form.
 * 
 * @param array $form (call by ref) 
 *   Description array of all form elements
 * @return void 
 */
function _menurolerights_menu_overview_form_submit_validate(&$form/*, &$form_state*/) {
  // Clear all messages
  drupal_get_messages(); // use dpm from here // dpm($form); // dpm($form_state);
  // Over all form elements ...
  foreach ($form as $data) {
    if ( // if $data is a menu item from main menu ...
        (isset($data['#item']) 
      && isset($data['#item']['menu_name']) 
      && ($data['#item']['menu_name'] == $form['menu_name']['#value']))
        // if parent of this menu item has changed ... 
      && (($data['plid']['#default_value'] != $data['plid']['#value']) 
        // The following doesn't work, so you have no form error message on 
        // rearranging menu items on same level. This is not an urgent problem!
        // || ($data['weight']['#default_value'] != $data['weight']['#value']) 
    )) {
      // drupal_set_message($data['weight']['#default_value'].' | '.$data['weight']['#value']);
      _menurolerights_sourceAndTargetItemAreSubchildsOfMasterItem($form, $data);
    }
  }
}

/**
 * Helper function for _menurolerights_menu_overview_form_submit_validate
 * 
 * @param array $form (call by ref) 
 *   Description array of all form elements
 * @param array $data 
 *   All data of a form element which should be tested if changed 
 * @return void 
 */
function _menurolerights_sourceAndTargetItemAreSubchildsOfMasterItem(&$form, $data) {
  $mlid = $data['#item']['mlid'];
  // Test on 'source item is a subchild of master_mlid'
  if (!_menurolerights_sourceItemIsSubchildOfMasterItem($form, $data)) {
    $form['mlid:' . $mlid]['plid']['#value'] = $data['plid']['#default_value'];
    form_set_error('wrong tree arrangement', t('Sie können diese(n) Manupunkt(e) nicht außerhalb der zulässigen Bereiche verschieben.'));
  }
  // Test on 'target item is a subchild of master_mlid'
  if (!_menurolerights_targetItemIsSubchildOfMasterItem($form, $data)) {
    $form['mlid:' . $mlid]['plid']['#value'] = $data['plid']['#default_value'];
    form_set_error('wrong tree arrangement', t('Sie können diese(n) Manupunkt(e) nicht aus den zulässigen Bereichen heraus verschieben.'));
  }
}

/**
 * Helper function for _menurolerights_sourceAndTargetItemAreSubchildsOfMasterItem
 * 
 * @param array $form (call by ref) 
 *   Description array of all form elements
 * @param array $data 
 *   All data of a form element which should be tested 
 * @return void 
 */
function _menurolerights_sourceItemIsSubchildOfMasterItem(&$form, $data) {
  $mlid = $data['#item']['mlid'];
  $isChild = false; 
  $next_parent = $mlid; // new parent
  while ($next_parent != 0) {
    // Test on 'next_parent is master_mlid'
    if ($next_parent == $form['master_mlid']['#value']) {
      $isChild = true;
      break;
    }
    // Get 'next_parent'
    if (isset($form['mlid:' . $next_parent])) {
      $next_parent = $form['mlid:' . $next_parent]['plid']['#default_value'];
    }
    else {
      break;
    }
  }
  return $isChild;
}

/**
 * Helper function for _menurolerights_sourceAndTargetItemAreSubchildsOfMasterItem
 * 
 * @param array $form (call by ref) 
 *   Description array of all form elements
 * @param array $data 
 *   All data of a form element which should be tested 
 * @return void 
 */
function _menurolerights_targetItemIsSubchildOfMasterItem(&$form, $data) {
  $isChild = false;
  $next_parent = $data['plid']['#value']; // new parent
  while ($next_parent != 0) {
    // Test on 'next_parent is master_mlid'
    if ($next_parent == $form['master_mlid']['#value']) {
      $isChild = true;
      break;
    }
    // Get 'next_parent'
    if (isset($form['mlid:' . $next_parent])) {
      $next_parent = $form['mlid:' . $next_parent]['plid']['#default_value'];
    }
    else {
      break;
    }
  }
}
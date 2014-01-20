<?php
/** 
 * menurolerights module include file  
 * 
 * Helper functions of form 'module settings'
 *
 * @author Wolfgang Hauertmann <info@hauertmann.com>
 * @copyright (c) 2014 Wolfgang Hauertmann
 * @license http://www.gnu.org/licenses/gpl-2.0.html GPL-2.0
 * 
 */

/**
 * Orders menu links hierarchically under a parent menu item 
 * 
 * @param array $list 
 *   unordered list of all menu items of a menu 
 * @param int $parent
 *   parent menu item id (plid)  
 * @param int $level
 *   hierarchical level indicator  
 * @return array 
 *   the ordered menu list under the parent item  
 */
function _hrmrr_order_menu_links($list, $parent=0, $level=0) {
  $res = array();
  foreach ($list as $value) {
    if($value['plid'] == $parent) {
      $value['level'] = $level;
      $res[] = $value;
      // Recursion
      $res = array_merge($res, _hrmrr_order_menu_links($list, $value['mlid'], $level+1));
    }
  }
  return $res;
}

/**
 * Get a list of menu titles indexed by internal menu-names
 * 
 * @return array 
 *   list of menu titles indexed by internal menu-names
 */
function _hrmrr_getListOfMenuTitles() {
  $menu_list = array('all' => t('All')); 
  $result = db_query("select * from ".$GLOBALS['databases']['default']['default']['prefix']."menu_custom");
  foreach ($result as $record) {
    $record = (array)$record;
    $menu_list[$record['menu_name']] = t($record['title']);
  }
  return $menu_list;  
}

/**
 * Get a list of rules
 * 
 * @return array 
 *   list of rules
 */
function _hrmrr_getListOfActiveRules() {
  $roles = user_roles(TRUE);
  $dbase_prefix = $GLOBALS['databases']['default']['default']['prefix'];
  // Get all active rules to array
  $rules = array();
  $result = db_query("select * from ".$dbase_prefix."menurolerights_rules");
  foreach ($result as $record) {
    $record = (array)$record;
    if ($record['active'] == 1) {
       $rules[] = array(
         'role' => $roles[$record['role_id']],
         'menu_name' => $record['menu_name'],
         'master_mlid' => $record['item_id'],
       );
    }
  }  
  return $rules;
}

/**
 * Get last selected menu name
 * 
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing. 
 * @return array 
 *   Last selected menu name
 */
function _hrmrr_getLastSelectedMenuName(&$form_state) {
  $menu_name = '';
  if (isset($form_state['storage']['settings']['values']['menurolerights_rule_menu_name'])) {
     $menu_name 
       = $form_state['storage']['settings']['values']['menurolerights_rule_menu_name'];
  } 
  return $menu_name;
}

/**
 * Get menu links of selected menu-name
 * 
 * @param string $menu_name 
 *   A menu name
 * @return array 
 *   Menu links of selected menu name
 */
function _hrmrr_getMenuLinksOfSelectedMenuName($menu_name) {
  $menu_links = array(); 
  if ($menu_name != '') {
    $result = db_query("select * from ".$GLOBALS['databases']['default']['default']['prefix']."menu_links where menu_name = '$menu_name' order by weight");
    $records = array();
    foreach ($result as $record) {
      $record = (array)$record;
      $records[] = $record;
    }
    $items = _hrmrr_order_menu_links($records);
    foreach ($items as $item) {
      $menu_links[$item['mlid']] = str_repeat("-", $item['level']).$item['link_title'];
    }
  }  
  return $menu_links;
}

/**
 * On first call return 1, on second call return 2
 * 
 * @param string $menu_name 
 *   A menu name
 * @param array $menu_links
 *   Menu links
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing. 
 * @return int 
 *   page state
 */
function _hrmrr_setPageState($menu_name, $menu_links, &$form_state) {
  $page = 1; 
  if(($menu_name=='all')||((count($menu_links) > 0) && (isset($form_state['storage']['page'])))) {
    $page = $form_state['storage']['page'];
  }
  return $page;
}

/**
 * Build form elements depending on page state
 * 
 * @param int $page 
 *   Page state: 1: first call, 2: second call
 * @param string $menu_name 
 *   A menu name, e.g. 'main-menu'
 * @param array $menu_list
 *   A string list of all Menu names
 * @param array $menu_links
 *   The menu links of the menu with given menu name
 * @param array $form (call by ref) 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing. 
 * @return void 
 */
function _hrmrr_buildFormElementsDependingOnPageState($page, $menu_name, $menu_list, $menu_links,&$form, &$form_state) {
  // Decide on 'first call' or 'second call'
  switch($page) {
    case 1: // First call
      // Build form elements for first call
      _hrmrr_buildFormElementsForFirstCall($menu_name, $menu_list, $menu_links,$form, $form_state);
      break;
  
    case 2: // Second call
      // Build form elements for second call
      _hrmrr_buildFormElementsForSecondCall($menu_name, $menu_list, $menu_links, $form, $form_state);
      break;
  }  
}

/**
 * Build form elements for first call
 * 
 * @param string $menu_name 
 *   A menu name, e.g. 'main-menu'
 * @param array $menu_list
 *   A string list of all Menu names
 * @param array $menu_links
 *   The menu links of the menu with given menu name
 * @param array $form (call by ref) 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing. 
 * @return void 
 */
function _hrmrr_buildFormElementsForFirstCall($menu_name, $menu_list, $menu_links, &$form, &$form_state){
  $form['first'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select a menu for adding a rule'),
  );
  // Menu selection element  
  $form['first']['menurolerights_rule_menu_name'] = array(
    '#type' => 'select',
    '#title' => 'Menu',
    '#options' => $menu_list,
    '#description' => t('Select a menu for which a rule for limited access of a role should be defined.'),
    '#default_value' => 0,
    '#required' => TRUE,
    // on selection submit form to the defined handler
    '#attributes' => array('onchange' => "form.submit('_menurolerights_form_submit')"),
  );
  // Attention: Very crude but ...
  // You need a hidden submit button for reaching the validation and submit function 
  // given for the 'onchange'-event of the select element abouve. 
  // Read this for explanations: https://drupal.org/node/292026
  $form['first']['go'] = array(
    '#type' => 'submit', 
    '#value' => t('Go'), 
    '#attributes' => array('style'=>'display:none')
  ); 
  // Set page state to '1th call'
  $form_state['storage']['page'] = 1;   
}



/**
 * Build form elements for second call
 * 
 * @param string $menu_name 
 *   A menu name, e.g. 'main-menu'
 * @param array $menu_list
 *   A string list of all Menu names
 * @param array $menu_links
 *   The menu links of the menu with given menu name
 * @param array $form (call by ref) 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing. 
 * @return void 
 */
function _hrmrr_buildFormElementsForSecondCall($menu_name, $menu_list, $menu_links, &$form, &$form_state){
  
  if ($menu_name == 'all') {
    $ftext = 'Modify access rule(s)';
  }
  else {
    $ftext = 'Add access rule for menu "'.$menu_list[$menu_name].'"';  
  }  
  $form['second'] = array(
    '#type' => 'fieldset',
    '#title' => t($ftext),
  ); 
  if ($menu_name != 'all') {  
    // Select box for roles
    $options1 = array(); 
    $result = db_query("select * from ".$GLOBALS['databases']['default']['default']['prefix']."role order by weight");
    $records = array();
    foreach ($result as $record) {
      $record = (array)$record;
      $options1[$record['rid']] = $record['name'];
    }
    $form['second']['rid'] = array(
      '#type' => 'select',
      '#title' => 'Role',
      '#options' => $options1,
      '#description' => t('Select a role which should have limited access to the menu abouve.'),
      '#default_value' => 3,
      '#required' => TRUE,
    );
    // Select box for menu items
    $form['second']['mlid'] = array(
      '#type' => 'select',
      '#title' => 'Role has access to menu "'.$menu_list[$menu_name] .'" under following item',
      '#default_value' => 0,
      '#options' => $menu_links,
      '#tree' => TRUE,
      '#description' => t('Select a menu item id under which the role abouve should have access to their submenu-items.'),
      '#required' => TRUE,
    );
  }  
  // Input element for a notice 
  $form['second']['notice'] = array(
    '#type' => 'textfield',
    '#title' => t('Notice'),
    '#default_value' => '',
    '#size' => 40,
    '#maxlength' => 40,
    '#description' => t('You can leave a notice here.'),
    '#required' => FALSE,
  );
  // Buttons
  $form['second']['actions'] = array('#type' => 'actions'); 
  $form['second']['actions']['save'] = array(
    '#type' => 'submit', 
    '#value' => t('Add')
  ); 
  $form['second']['actions']['cancel'] = array(
    '#markup' => l(t('Cancel'), 'admin/config/content/menurolerights'),
  );  
}

/**
 * Build table of defined rules
 * 
 * @param string $menu_name 
 *   A menu name, e.g. 'main-menu'
 * @param array $menu_list
 *   A string list of all Menu names
 * @param int $page 
 *   Page state: 1: first call, 2: second call
 * @param array $form (call by ref) 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing. 
 * @return void 
 */
function _hrmrr_buildTableOfDefinedRules($menu_name, $menu_list, $page, &$form, &$form_state) {
  $form['table'] = array(
     '#type' => 'fieldset',
     '#title' => t('List of rules allowing roles to access to the definied menu areas if active'),
  );
  $roles = user_roles(true);
  // get rules for the selected menu or if not given get all rules
  $where = '';
  if ($menu_name != '') {
      if ($menu_name != 'all') {
         $where = " where menu_name = '$menu_name' ";
      }
      else {
         $where = "";
         foreach($form_state['storage']['settings']['values']['table'] as $key=>$val) {
           if ($val!=0) {
             if ($where=='') {
                $where = " where id=$key ";
             }
             else {
                $where .= " OR id=$key "; 
             }
           }
         }
      }
  }
  $result = db_query("select * from ".$GLOBALS['databases']['default']['default']['prefix']."menurolerights_rules $where order by active desc, role_id, menu_name");
  $records = array();
  foreach ($result as $record) {
    $record = (array)$record;
    $records[] = $record;
  }  
  // Fill array with menu rules
  $menu_rules = array(); 
  foreach ($records as $rule) {
    $res = db_query("select * from ".$GLOBALS['databases']['default']['default']['prefix']."menu_links where mlid = $rule[item_id]");
    $recs = array();
    foreach ($res as $rec) {
      $record = (array)$rec;
      $recs[] = $rec;
    }
    $mitem = (array)$recs[0];
    $menu_rules[$rule['id']] = array(
      'role' => $roles[$rule['role_id']],
      'menu_title' => $menu_list[$rule['menu_name']],  // nicht t(), weil sonst doppelt übersetzt
      'menu_item' => $mitem['link_title']." (id=$rule[item_id])",
      'active' => $rule['active']==1?t('Yes'):t('No'),
      'notice' => $rule['notice'],
      'upd_user' => $rule['upd_user'],
      'upd_date' => date("d.m.Y - H:i", strtotime($rule['upd_date'])),
    );
  }
  // Tableselect element filled with rules 
  $header = array(
    'role' => t('Role'),
    'menu_title' => t('Menu'),
    'menu_item' => t('Menu item under which role has access'),
    'active' => t('Aktiv'),
    'notice' => t('Notice'),
    'upd_user' => t('From'),
    'upd_date' => t('Updated'),    
  );
  $form['table']['table'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $menu_rules,
    '#empty' => t('No content available.'),
  );
  if ($page == 1) {
    // Select box filled with actions for manipulating the selecttable
    $form['table']['sel_tab_action'] = array(
      '#type' => 'select',
      '#title' => t('Selection'),
      '#options' => array(
        SEL_NONE     => t('<none>'),
        SEL_DELETE   => t('Delete'),
        SEL_ACTIVE   => t('Set rule active'),
        SEL_INACTIVE => t('Set rule inactive'),
        SEL_CHANGE   => t('Change notice'),
      ),
      '#default_value' => 0,
      '#description' => t('Set this to <em>Yes</em> if you would like this category to be selected by default.'),
      '#attributes' => array('onchange' => "form.submit('_menurolerights_form_submit')"),
    );
  }
  else {
    if ($menu_name == 'all') {  
      $form['sel_tab_action'] = array(
        '#type' => 'hidden',
        '#value' => SEL_CHANGE,
      );
    }
  }
}

/**
 * Handle selection 'no action' on submit of formular for module settings
 * 
 * This selection happens, if you come from 2th page state ('rule saved' or 'canceled')
 * 
 * @param array $form 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing.
 * @return void 
 */
function _hrmrr_menurolerights_form_submit_SEL_NONE($form, &$form_state) {
  global $user;
  // The stored page state must be defined here
  if (!isset($form_state['storage']['page'])) { 
    $form_state['storage']['page'] = 1;
  }
  // On 1th page state store form values and set page state to 2th. 
  if ($form_state['storage']['page'] == 1) {
    $form_state['rebuild'] = TRUE;
    $form_state['storage']['settings']['values'] = $form_state['values'];
    $form_state['storage']['page']++;
  }
  /* Save rule */
  else {
    // Reset page state in the form storage
    $form_state['rebuild'] = FALSE;
    unset($form_state['storage']['page']);
    // Get selected menu-name from form state  
    $menu_name = _hrmrr_getLastSelectedMenuName($form_state); 
    // Insert rule
    $tstamp = date('Y-m-d H:i:s');
    $nid = db_insert('menurolerights_rules')
      ->fields(array(
        'active' => true,
        'menu_name' => $menu_name,
        'role_id' => $form_state['values']['rid'],
        'item_id' => $form_state['values']['mlid'],
        'notice' => $form_state['values']['notice'],
        'cre_user' => $user->name,
        'cre_date' => $tstamp,
        'upd_user' => $user->name,
        'upd_date' => $tstamp,
      ))
      ->execute();
    // Finish 
    drupal_set_message('Rule saved', 'status'); 
    drupal_goto('admin/config/content/menurolerights');  
  }   
}

/**
 * Handle selection 'delete rule' on 1th call submit of formular for module settings
 * 
 * @param array $form 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing.
 * @return void 
 */
function _hrmrr_menurolerights_form_submit_SEL_DELETE($form, &$form_state) {
  // Over all items in the rule table ...
  $count = 0;  
  foreach($form_state['values']['table'] as $id => $val) {
    // if not checked: continue
    if ($val == 0) {
      continue;  
    }
    // else delete rule
    $num_deleted = db_delete('menurolerights_rules')
      ->condition('id', $id)
      ->execute();
    $count = $count + $num_deleted;
  }
  // Finish
  $form_state['storage']['page'] = 0;
  drupal_set_message($count.' rule(s) deleted', 'status'); 
  drupal_goto('admin/config/content/menurolerights');    
}

/**
 * Handle selection 'set rule active' on 1th call submit of formular for module settings
 * 
 * @param array $form 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing.
 * @return void 
 */
function _hrmrr_menurolerights_form_submit_SEL_ACTIVE($form, &$form_state) {
  // Over all items in the rule table ...
  $count = 0;  
  foreach($form_state['values']['table'] as $id => $val) {
    // if not checked: continue
    if ($val == 0) {
      continue;  
    }
    // else set rule active
    $num_updated = db_update('menurolerights_rules')
      ->fields(array(
        'active' => 1,
        ))
      ->condition('id', $id)
      ->execute();
    $count = $count + $num_updated;
  }
  // Finish
  $form_state['storage']['page'] = 0;
  drupal_set_message($count.' rule(s) activated', 'status'); 
  drupal_goto('admin/config/content/menurolerights');   
}

/**
 * Handle selection 'set rule inactive' on 1th call submit of formular for module settings
 * 
 * @param array $form 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing.
 * @return void 
 */
function _hrmrr_menurolerights_form_submit_SEL_INACTIVE($form, &$form_state) {
  // Over all items in the rule table ...
  $count = 0;  
  foreach($form_state['values']['table'] as $id => $val) {
    // if not checked: continue
    if ($val == 0) {
      continue;  
    }
    // else set rule inactive
    $num_updated = db_update('menurolerights_rules')
      ->fields(array(
        'active' => 0,
        ))
      ->condition('id', $id)
      ->execute();
    $count = $count + $num_updated;
  }
  // Finish
  $form_state['storage']['page'] = 0;
  drupal_set_message($count.' rule(s) deactivated', 'status'); 
  drupal_goto('admin/config/content/menurolerights');   
}

/**
 * Handle selection 'change notice' on 1th call submit of formular for module settings
 * 
 * @param array $form 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing.
 * @return void 
 */
function _hrmrr_menurolerights_form_submit_SEL_CHANGE($form, &$form_state) {
  // On 1th page state store form values and set page state to 2th. 
  if(!isset($form_state['storage']['page'])||($form_state['storage']['page'] == 1)) {
    $form_state['rebuild'] = TRUE;
    $form_state['storage']['settings']['values'] = $form_state['values'];
    // $form_state['storage']['page']++;
    $form_state['storage']['page'] = 2;
  }
  /* Change notice */
  else {
    // Reset page state in the form storage
    $form_state['rebuild'] = FALSE;
    unset($form_state['storage']['page']);
    // Over all items in the rule table ...
    $count = 0;  
    foreach($form_state['values']['table'] as $id => $val) {
      // if not checked: continue
      if ($val == 0) {
        continue;  
      }
      // else update notice
      $num_updated = db_update('menurolerights_rules')
        ->fields(array(
          'notice' => $form_state['values']['notice'],
          ))
        ->condition('id', $id)
        ->execute();
      $count = $count + $num_updated;
    }
    // Finish
    $form_state['storage']['page'] = 0;
    drupal_set_message($count.' rule(s) notice updated', 'status'); 
    drupal_goto('admin/config/content/menurolerights');         
  }
}



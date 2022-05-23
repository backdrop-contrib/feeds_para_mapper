<?php
/**
 * @file
 * FeedsTermProcessor class.
 */

/**
 * Feeds processor plugin. Createparagraphs from feed items.
 */
class FeedsParagraphProcessor extends FeedsProcessor {

  /**
   * Define entity type.
   */
  public function entityType() {
    return 'paragraphs_item';
  }

  /**
   * Implements parent::entityInfo().
   */
  protected function entityInfo() {
    $info = parent::entityInfo();
    $info['entity keys']['bundle'] = 'paragraphs_item';
    $info['label plural'] = t('Paragraphs');
    $info['bundle name'] = t('Paragraph');
    return $info;
  }

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    return array(
      'host_entity_type' => 'node',
      'field_name' => '',
    ) + parent::configDefaults();
  }

  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = parent::configForm($form_state);

    $all_fields = field_info_fields();
    $field_options = array();
    $entity_options = array();
    foreach ($all_fields as $field) {
      if ($field['type'] == 'paragraphs') {
        foreach ($field['bundles'] as $entity_type => $bundles) {
          $entity_options[$entity_type] = $entity_type;
          foreach ($bundles as $bundle) { 
            $field_info = field_info_instance($entity_type, $field['field_name'], $bundle);
            $field_options[$field['field_name']] = $field_info['label'] . ' (' . $field['field_name'] . ')';
          }
        }
      }
    }

    $form['host_entity_type'] = array(
      '#type' => 'select',
      '#options' => $entity_options,
      '#title' => t('Host Entity Type'),
      '#description' => t('The type of the entity that references this paragraph, for example: "node".'),
      '#default_value' => $this->config['host_entity_type'],
      '#required'=> TRUE,
      '#weight' => -9,
    );
    $form['field_name'] = array(
      '#type' => 'select',
      '#options' => $field_options,
      '#title' => t('Paragraph Reference Field'),
      '#description' => t('Select the paragraph field on the parent node.'),
      '#default_value' => $this->config['field_name'],
      '#required'=> TRUE,
      '#weight' => -8,
    );

    $form['bundle']['#weight'] = -10;

    return $form;
  }

  /**
   * Override parent::configFormValidate().
   */
  public function configFormValidate(&$values) {
    // @todo
  }

  /**
   * Reschedule if expiry time changes.
   */
  public function configFormSubmit(&$values) {
    parent::configFormSubmit($values);
  }

  /**
   * Creates a new paragraph in memory and returns it.
   */
  protected function newEntity(FeedsSource $source) {
    $paragraph = parent::newEntity($source);
    $paragraph->bundle = $this->config['bundle'];
    $paragraph->field_name = $this->config['field_name'];

    return $paragraph;
  }

  /**
   * Load an existing entity.
   */
  protected function entityLoad(FeedsSource $source, $entity_id) {
    $entity = parent::entityLoad($source, $entity_id);

    return $entity;
  }

  /**
   * Saves a paragraph.
   *
   * We de-array parent fields with only one item.
   * This stops leftandright module from freaking out.
   */
  protected function entitySave($paragraph) {
    if (isset($paragraph->host_entity_id)) {
      // @todo det this from config settings.
      $entity_type = 'node';
      $entity = entity_load($entity_type, $paragraph->host_entity_id);
      $paragraph->setHostEntity($entity_type, $entity);

      dpm($paragraph);
      // Do we need validation here?
      if (isset($paragraph->id) && ($paragraph->parent == $paragraph->id || (is_array($paragraph->parent) 
            && in_array($paragraph->id, $paragraph->parent)))
      ) {
        // @todo trow validation error
      }

      $paragraph->save();
    }
  }

  /**
   * Deletes a series of terms.
   */
  protected function entityDeleteMultiple($pids) {
    // @todo lookup sybtax
    entity_delete('paragraphs_item', $pids);
  }

  /**
   * Overrides parent::setTargetElement().
   *
   * Operate on a target item that is a paragraph item.
   */
  public function setTargetElement(FeedsSource $source, $target_paragraph, $target_element, $value, array $mapping = array()) {
    switch ($target_element) {
      case 'host_entity_id':
        if (!empty($value)) {
          $target_paragraph->host_entity_id = $value;
        }
        break;

      default:
        parent::setTargetElement($source, $target_paragraph, $target_element, $value);
        break;
    }
  }

  /**
   * Return available mapping targets.
   */
  public function getMappingTargets() {
    $targets = parent::getMappingTargets();

    $targets['host_entity_id'] = array(
      'name' => t('Host Entity ID (required)'),
      'description' => t('The ID of the entity that references this paragraph.'),
    );
    $targets['entity_id'] = array(
      'name' => t('Paragraph Entity ID'),
      'description' => t('The ID of the paragrpah. NOTE: use this feature with care, entity ids are usually assigned by Backdrop.'),
      'optional_unique' => TRUE,
    );

    $this->getHookTargets($targets);

    return $targets;
  }

  /**
   * Overrides FeedsProcessor::dependencies().
   */
  public function dependencies() {
    $dependencies = parent::dependencies();
    $dependencies['paragraphs'] = 'paragraphs';
    return $dependencies;
  }

}

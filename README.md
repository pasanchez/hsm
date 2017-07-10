# HSM

Home Stock Manager

Small web-based app to manage house stock, recipes and shopping lists.

## REST API:

* n:
  * Create new Item
  * params: name stock min_stock max_stock unit
* s:
  * Search by name in DB
  * params: name 
* list:
  * Outputs shopping list
* d:
  * Delete item by ID in DB
  * params: ID 
* u:
  * Update item stock by ID in DB
  * params: ID value

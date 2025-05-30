import json
import boto3
from datetime import datetime

dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table('ElevatorState')

def lambda_handler(event, context):
    required_fields = ['device_id', 'current_floor', 'selected_floor']
    for field in required_fields:
        if field not in event:
            raise Exception(f"Missing field: {field}")

    item = {
        'device_id': event['device_id'],
        'current_floor': int(event['current_floor']),
        'selected_floor': int(event['selected_floor']),
        'timestamp': datetime.utcnow().isoformat()
    }

    table.put_item(Item=item)

    return {
        'statusCode': 200,
        'body': json.dumps({'message': 'State saved successfully'})
    }
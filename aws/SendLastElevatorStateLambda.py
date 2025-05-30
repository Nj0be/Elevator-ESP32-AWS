import json
import boto3
from decimal import Decimal
from boto3.dynamodb.conditions import Key

dynamodb = boto3.resource('dynamodb')
iot = boto3.client('iot-data', region_name='<YOUR-REGION>')
table = dynamodb.Table('ElevatorState')

class DecimalEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, Decimal):
            return int(obj) if obj % 1 == 0 else float(obj)
        return super(DecimalEncoder, self).default(obj)

def lambda_handler(event, context):
    topic = event.get('topic', '')  # es: 'elevator/<device_id>/sub'
    device_id = topic.split('/')[-2]
    if not device_id:
        raise Exception("Missing device_id in topic")

    response = table.query(
        KeyConditionExpression=Key('device_id').eq(device_id),
        ScanIndexForward=False,
        Limit=1
    )

    items = response.get('Items', [])
    if not items:
        return {'statusCode': 404, 'body': json.dumps({'error': 'No state found'})}

    last_state = items[0]

    iot.publish(
        topic=f'elevator/{device_id}/sub',
        qos=1,
        payload=json.dumps(last_state, cls=DecimalEncoder)
    )

    return {
        'statusCode': 200,
        'body': json.dumps({'message': 'State sent to device'})
    }